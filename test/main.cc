#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include "catch.hpp"
#include "forcelayout.cc/layout.h"
#include "nangraph.cc/graph.h"
#include "quadtree.cc/quadtree.h"

TEST_CASE("it can make a step", "[layout]" ) {
  Graph graph;
  auto linkId = graph.addLink(1, 2);

  ForceLayout layout(graph);
  auto movement = layout.step();
  REQUIRE(movement > 0);
}

TEST_CASE("it can return a body", "[layout]" ) {
  Graph graph;
  graph.addLink(1, 2);

  ForceLayout layout(graph);
  layout.step();

  Body *first = layout.getBody(1);
  REQUIRE(first != nullptr);
  REQUIRE(first->pos.x != 0);
  REQUIRE(first->pos.y != 0);
  REQUIRE(first->pos.z != 0);

  Body *second = layout.getBody(2);
  REQUIRE(second != nullptr);
  REQUIRE(second->pos.x != 0);
  REQUIRE(second->pos.y != 0);
  REQUIRE(second->pos.z != 0);

  Body *none = layout.getBody(42);
  REQUIRE(none == nullptr);
}

TEST_CASE("it can step with no links present", "[layout]" ) {
  Graph graph;
  graph.addNode(1);
  graph.addNode(2);

  ForceLayout layout(graph);
  layout.step();

  Body *first = layout.getBody(1);
  REQUIRE(first != nullptr);
  REQUIRE(first->pos.x != 0);
  REQUIRE(first->pos.y != 0);
  REQUIRE(first->pos.z != 0);

  Body *second = layout.getBody(2);
  REQUIRE(second != nullptr);
  REQUIRE(second->pos.x != 0);
  REQUIRE(second->pos.y != 0);
  REQUIRE(second->pos.z != 0);
}
