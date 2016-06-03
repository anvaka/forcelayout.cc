#include "forcelayout.cc/layout.h"

double ForceLayout::step() {
  accumulate();

  double totalMovement = integrate();
  return totalMovement;
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
  Body *fromBody = nullptr;
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
  double dx = 0, tx = 0,
  dy = 0, ty = 0,
  dz = 0, tz = 0,
  timeStep = _settings.timeStep;

#pragma omp parallel for
  for (auto body : _bodies) {
    double coeff = timeStep / body->mass;

    body->velocity.x += coeff * body->force.x;
    body->velocity.y += coeff * body->force.y;
    body->velocity.z += coeff * body->force.z;

    double vx = body->velocity.x,
    vy = body->velocity.y,
    vz = body->velocity.z,
    v = sqrt(vx * vx + vy * vy + vz * vz);

    if (v > 1) {
      body->velocity.x = vx / v;
      body->velocity.y = vy / v;
      body->velocity.z = vz / v;
    }

    dx = timeStep * body->velocity.x;
    dy = timeStep * body->velocity.y;
    dz = timeStep * body->velocity.z;

    body->pos.x += dx;
    body->pos.y += dy;
    body->pos.z += dz;

    tx += abs(dx); ty += abs(dy); tz += abs(dz);
  }

  return (tx * tx + ty * ty + tz * tz)/_bodies.size();
}

void ForceLayout::updateDragForce(Body *body) {
  body->force.x -= _settings.dragCoeff * body->velocity.x;
  body->force.y -= _settings.dragCoeff * body->velocity.y;
  body->force.z -= _settings.dragCoeff * body->velocity.z;
}

void ForceLayout::updateSpringForce(Body *source) {
  Body *body1 = source;

  for (auto body2 : source->springs) {
    double dx = body2->pos.x - body1->pos.x;
    double dy = body2->pos.y - body1->pos.y;
    double dz = body2->pos.z - body1->pos.z;
    double r = sqrt(dx * dx + dy * dy + dz * dz);

    if (r == 0) {
      dx = (random.nextDouble() - 0.5) / 50;
      dy = (random.nextDouble() - 0.5) / 50;
      dz = (random.nextDouble() - 0.5) / 50;
      r = sqrt(dx * dx + dy * dy + dz * dz);
    }

    double d = r - _settings.springLength;
    double coeff = _settings.springCoeff * d / r;

    body1->force.x += coeff * dx;
    body1->force.y += coeff * dy;
    body1->force.z += coeff * dz;

    body2->force.x -= coeff * dx;
    body2->force.y -= coeff * dy;
    body2->force.z -= coeff * dz;
  }
}
