#include "forcelayout.cc/layout.h"

#include <numeric>

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

  NodeCallback initBody = [&](const std::size_t& nodeId) -> bool {
    auto body = new Body();
    auto node = graph.getNode(nodeId);
    auto inEdgesCount = node->inNodes.size();
    auto outEdgesCount = node->outNodes.size();
    body->mass = 1 + (inEdgesCount + outEdgesCount)/3.0;
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

Body *ForceLayout::getBody(const std::size_t &nodeId) {
  auto search = _nodeIdToBody.find(nodeId);
  if (search == _nodeIdToBody.end()) {
    return nullptr;
  }
  return search->second;
}

void ForceLayout::setInitialPositions() {
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
    Vector3 base(0, 0, 0);
    for (auto other: body->springs) {
      base.add(other->pos);
    }
    int neighboursSize = body->springs.size();
    if (neighboursSize > 0) {
      base.multiplyScalar(1./neighboursSize);
    }

    int springLength = _settings.springLength;
    Vector3 offset(springLength * (random.nextDouble() - 0.5),
                   springLength * (random.nextDouble() - 0.5),
                   springLength * (random.nextDouble() - 0.5));
    body->pos.set(base)->add(offset);
  }
}

void ForceLayout::accumulate() {
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

double ForceLayout::integrate() {
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
