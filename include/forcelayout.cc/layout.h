//
//  layout.h
//  forcelayout.cc
//
//  Created by Andrei Kashcha on 5/30/16.
//  Copyright (c) 2015 Andrei Kashcha. All rights reserved.
//

#ifndef __layout__
#define __layout__

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <numeric>

#include "nangraph.cc/graph.h"
#include "random.cc/random.h"
#include "quadtree.cc/quadtree.h"

class BodyNotFoundException: public exception {};

struct LayoutSettings {
  double stableThreshold;
  double gravity;
  double theta;
  double dragCoeff;
  double springCoeff;
  double springLength;
  double timeStep;
  
  LayoutSettings() {
    stableThreshold = 0.009;
    gravity = -1.2;
    theta = 0.8;
    dragCoeff = 0.02;
    springCoeff = 0.0008;
    springLength = 30;
    timeStep = 20;
  }
  
  LayoutSettings& operator =(const LayoutSettings &other) {
    stableThreshold = other.stableThreshold;
    gravity = other.gravity;
    theta = other.theta;
    dragCoeff = other.dragCoeff;
    springCoeff = other.springCoeff;
    springLength = other.springLength;
    timeStep = other.timeStep;
    return *this;
  }
};

template <size_t N>
class ForceLayout {
  Graph &graph;
  LayoutSettings _settings;
  Random random;
  std::vector<Body<N>*> _bodies;
  // this maps nodeId to Body address. TODO: Find a way to not use this.
  std::unordered_map<std::size_t, Body<N>*> _nodeIdToBody;

  QuadTree<N> tree;

  void accumulate() {
    tree.insertBodies(_bodies);
    auto bodiesCount = _bodies.size();

#pragma omp parallel for
    for (std::size_t i = 0; i < bodiesCount; ++i) {
      auto body = _bodies[i];
      body->force.reset();

      tree.updateBodyForce(body);
      updateDragForce(body);
    }

#pragma omp parallel for
    for (std::size_t i = 0; i < bodiesCount; ++i) {
      auto body = _bodies[i];
      updateSpringForce(body);
    }
  }

  double integrate() {
    double timeStep = _settings.timeStep;
    double totalV = 0;
    auto bodiesCount = _bodies.size();

#pragma omp parallel for
    for (std::size_t i = 0; i < bodiesCount; ++i) {
      auto body = _bodies[i];
      double coeff = timeStep / body->mass;

      body->velocity.addScaledVector(body->force, coeff);

      double v = body->velocity.length();
      totalV += v;

      if (v > 1) body->velocity.normalize();

      body->pos.addScaledVector(body->velocity, timeStep);
    }
    
    return totalV/_bodies.size();

  }
  void updateDragForce(Body<N> *body) {
    body->force.addScaledVector(body->velocity, -_settings.dragCoeff);
  }
  void updateSpringForce(Body<N> *source) {
    Body<N> *body1 = source;

    for (auto body2 : source->springs) {
      Vector3<N> dist = body2->pos - body1->pos;
      double r = dist.length();

      if (r == 0) {
        for (size_t i = 0; i < N; ++i) {
          dist.coord[i] = (random.nextDouble() - 0.5) / 50;
        }
        r = dist.length();
      }

      double coeff = _settings.springCoeff * (r - _settings.springLength) / r;

      body1->force.addScaledVector(dist, coeff);
      body2->force.addScaledVector(dist, -coeff);
    }

  }
  void initBodies() {
    _bodies.reserve(graph.getNodesCount());

    NodeCallback initBody = [&](const std::size_t& nodeId) -> bool {
      auto body = new Body<N>();
      auto node = graph.getNode(nodeId);
      auto degree = node->degree();
      body->mass = 1 + degree/3.0;
      _bodies.push_back(body);
      _nodeIdToBody[nodeId] = body; // TODO: Find a way to remove this and save ram.
      return false;
    };

    graph.forEachNode(initBody);

    // Now that we have bodies, let's add links:
    LinkCallback initSpring = [&](const std::size_t& fromId, const std::size_t& toId, const std::size_t& linkId) -> bool {
      // TODO: Add verification
      _nodeIdToBody[fromId]->springs.push_back(_nodeIdToBody[toId]);
      return false;
    };
    graph.forEachLink(initSpring);
    setInitialPositions();
  }

  void setInitialPositions() {
    // TODO: I think this can be done better. For example, combine
    // CW algorithm with initial placement?
    auto nodesCount = _bodies.size();

    std::vector<int> degree(nodesCount);
    std::iota(degree.begin(), degree.end(), 0);
    std::sort(degree.begin(), degree.end(),
              [&](const int &fromBody, const int &toBody) -> bool {
                return _bodies[fromBody]->springs.size() > _bodies[toBody]->springs.size();
              });

    for (auto body : _bodies){
      Vector3<N> base;
      for (auto other: body->springs) {
        base.add(other->pos);
      }
      int neighboursSize = body->springs.size();
      if (neighboursSize > 0) {
        base.multiplyScalar(1./neighboursSize);
      }

      int springLength = _settings.springLength;
      Vector3<N> offset;
      for (size_t i = 0; i < N; ++i) {
        offset.coord[i] = springLength * (random.nextDouble() - 0.5);
      }
      body->pos.set(base)->add(offset);
    }
  }
  
  static LayoutSettings makeDefaultSettings() {
    LayoutSettings settings;
    return settings;
  }

public:
  ForceLayout(Graph &g, LayoutSettings settings) :
    graph(g),
    _settings(settings),
    random(42) {
    initBodies();
  }
  
  ForceLayout(Graph &g) : ForceLayout(g, ForceLayout::makeDefaultSettings())  {}
  
  ~ForceLayout() {
    for (auto &i: _bodies) {
      delete i;
    }
  }

  /**
   * Sets a position for body. If no such body exist, throws BodyNotFoundException.
   */
  void setPosition(const std::size_t &bodyId, const Vector3<N> &position) {
    auto body = getBody(bodyId);
    if (!body) {
      throw BodyNotFoundException();
    }
    body->pos.set(position);
  }

  /**
   * Performs one iteration of force layout. Returns total movement performed
   * during that step.
   */
  double step() {
    accumulate();

    double totalMovement = integrate();
    return totalMovement;
  }
  
  /**
   * Given a node id from a graph, returns a pointer to its body object or
   * null if no such node is found.
   */
  Body<N> *getBody(const std::size_t &nodeId) {
    auto search = _nodeIdToBody.find(nodeId);
    if (search == _nodeIdToBody.end()) {
      return nullptr;
    }
    return search->second;
  }
  
  QuadTree<N> *getTree() { return &tree; }
};

#endif
