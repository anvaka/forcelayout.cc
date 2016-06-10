#include "forcelayout.cc/layout.h"

#include <iostream>

double ForceLayout::step() {
  accumulate();

  double totalMovement = integrate();
  return totalMovement;
}

void ForceLayout::setPosition(const std::size_t &bodyId, const Vector3 &position) {
  auto body = getBody(bodyId);
  if (!body) {
    throw BodyNotFoundException();
  }
  body->pos.set(position);
}

void ForceLayout::initBodies() {
  _bodies.reserve(graph.getNodesCount());

  NodeCallback initBody = [=](const std::size_t& nodeId) -> bool {
    auto body = new Body();
    _bodies.push_back(body);
    _nodeIdToBody[nodeId] = body; // TODO: Find a way to remove this and save ram.
    return false;
  };

  graph.forEachNode(initBody);

  // Now that we have bodies, let's add links:
  LinkCallback initSpring = [=](const std::size_t& fromId, const std::size_t& toId, const std::size_t& linkId) -> bool {
    // TODO: Add verification
    _nodeIdToBody[fromId]->springs.push_back(_nodeIdToBody[toId]);
    return false;
  };
  graph.forEachLink(initSpring);
}

Body *ForceLayout::getBody(const std::size_t &nodeId) {
  auto search = _nodeIdToBody.find(nodeId);
  if (search == _nodeIdToBody.end()) {
    return nullptr;
  }
  return search->second;
}

void ForceLayout::accumulate() {
  tree.insertBodies(_bodies);

  #pragma omp parallel for
  for (auto body: _bodies) {
    body->force.reset();

    tree.updateBodyForce(body);
    updateDragForce(body);
  }

#pragma omp parallel for
  for (auto body: _bodies) {
    updateSpringForce(body);
  }

}

double ForceLayout::integrate() {
  double timeStep = _settings.timeStep;
  double totalV = 0;

#pragma omp parallel for
  for (auto body : _bodies) {
    double coeff = timeStep / body->mass;

    body->velocity.addScaledVector(body->force, coeff);

    double v = body->velocity.length();
    totalV += v;

    if (v > 1) body->velocity.normalize();

    body->pos.addScaledVector(body->velocity, timeStep);
  }

  return totalV/_bodies.size();
}

void ForceLayout::updateDragForce(Body *body) {
  body->force.addScaledVector(body->velocity, -_settings.dragCoeff);
}

void ForceLayout::updateSpringForce(Body *source) {
  Body *body1 = source;

  for (auto body2 : source->springs) {
    Vector3 dist = body2->pos - body1->pos;
    double r = dist.length();

    if (r == 0) {
      dist.set((random.nextDouble() - 0.5) / 50,
               (random.nextDouble() - 0.5) / 50,
               (random.nextDouble() - 0.5) / 50);
      r = dist.length();
    }

    double coeff = _settings.springCoeff * (r - _settings.springLength) / r;

    body1->force.addScaledVector(dist, coeff);
    body2->force.addScaledVector(dist, -coeff);
  }
}
