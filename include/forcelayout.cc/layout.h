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

class ForceLayout {
  Graph &graph;
  Random random;
  // this maps nodeId to Body address
  std::vector<Body*> _bodies;
  std::unordered_map<std::size_t, Body*>  _nodeIdToBody;

  QuadTree tree;

  void accumulate();
  double integrate();
  void updateDragForce(Body *body);
  void updateSpringForce(Body *source);
  void initBodies();

public:
  ForceLayout(Graph &g): graph(g), random(42) {
    initBodies();
  }
  
  ~ForceLayout() {
    for (auto &i: _bodies) {
      delete i;
    }
  }
  
  /**
   * Performs one iteration of force layout
   */
  double step();
  Body *getBody(const std::size_t &nodeId);
};

#endif
