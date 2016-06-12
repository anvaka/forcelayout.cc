#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include "catch.hpp"
#include "forcelayout.cc/layout.h"
#include "nangraph.cc/graph.h"
#include "quadtree.cc/quadtree.h"

TEST_CASE("it can make a step in", "[layout 3d]" ) {
  Graph graph;
  auto linkId = graph.addLink(1, 2);

  ForceLayout<3> layout(graph);
  auto movement = layout.step();
  REQUIRE(movement > 0);
}

TEST_CASE("it can set body position", "[layout 3d]" ) {
  Graph graph;
  graph.addLink(1, 2);

  ForceLayout<3> layout(graph);
  Vector3<3> position;
  position.coord[0] = 0;
  position.coord[1] = 1;
  position.coord[2] = 0;
  layout.setPosition(1, position);
  auto body = layout.getBody(1);
  REQUIRE(body->pos == position);
}

TEST_CASE("it can return a body", "[layout 3d]" ) {
  Graph graph;
  graph.addLink(1, 2);

  ForceLayout<3> layout(graph);
  layout.step();

  auto first = layout.getBody(1);
  REQUIRE(first != nullptr);
  REQUIRE(first->pos.coord[0] != 0);
  REQUIRE(first->pos.coord[1] != 0);
  REQUIRE(first->pos.coord[2] != 0);

  auto second = layout.getBody(2);
  REQUIRE(second != nullptr);
  REQUIRE(second->pos.coord[0] != 0);
  REQUIRE(second->pos.coord[1] != 0);
  REQUIRE(second->pos.coord[2] != 0);

  auto none = layout.getBody(42);
  REQUIRE(none == nullptr);
}

TEST_CASE("it can step with no links present", "[layout 3d]" ) {
  Graph graph;
  graph.addNode(1);
  graph.addNode(2);

  ForceLayout<3> layout(graph);
  layout.step();

  auto first = layout.getBody(1);
  REQUIRE(first != nullptr);
  REQUIRE(first->pos.coord[0] != 0);
  REQUIRE(first->pos.coord[1] != 0);
  REQUIRE(first->pos.coord[2] != 0);

  auto second = layout.getBody(2);
  REQUIRE(second != nullptr);
  REQUIRE(second->pos.coord[0] != 0);
  REQUIRE(second->pos.coord[1] != 0);
  REQUIRE(second->pos.coord[2] != 0);
}

TEST_CASE("2d: it can step with no links present", "[layout 2d]" ) {
  Graph graph;
  graph.addNode(1);
  graph.addNode(2);

  ForceLayout<2> layout(graph);
  layout.step();

  auto first = layout.getBody(1);
  REQUIRE(first != nullptr);
  REQUIRE(first->pos.coord[0] != 0);
  REQUIRE(first->pos.coord[1] != 0);

  auto second = layout.getBody(2);
  REQUIRE(second != nullptr);
  REQUIRE(second->pos.coord[0] != 0);
  REQUIRE(second->pos.coord[1] != 0);
}