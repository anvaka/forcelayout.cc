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
#include "nangraph/graph.h"
#include "random.cc/random.h"
#include "quadtree.cc/quadtree.h"

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


class ForceLayout {
  Graph &graph;
  LayoutSettings _settings;
  Random random;
  std::vector<Body*> _bodies;
  // this maps nodeId to Body address. TODO: Find a way to not use this.
  std::unordered_map<std::size_t, Body*>  _nodeIdToBody;

  QuadTree tree;

  void accumulate();
  double integrate();
  void updateDragForce(Body *body);
  void updateSpringForce(Body *source);
  void initBodies();
  
  static LayoutSettings makeDefaultSettings() {
    LayoutSettings settings;
    return settings;
  }
public:
  ForceLayout(Graph &g, LayoutSettings settings) :
    graph(g),
    random(42),
    _settings(settings) {
    initBodies();
  }
  
  ForceLayout(Graph &g) : ForceLayout(g, ForceLayout::makeDefaultSettings())  {}
  
  ~ForceLayout() {
    for (auto &i: _bodies) {
      delete i;
    }
  }
  
  /**
   * Performs one iteration of force layout. Returns total movement performed
   * during that step.
   */
  double step();
  
  /**
   * Given a node id from a graph, returns a pointer to its body object or
   * null if no such node is found.
   */
  Body *getBody(const std::size_t &nodeId);
};

#endif
