//==========================================================================
// ViGraph dataflow module: vector/collision-detector/test-collision-detector.cc
//
// Tests for <collision-detector> filter
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include "../../module-test.h"

class CollisionDetectorTest: public GraphTester
{
public:
  CollisionDetectorTest()
  {
    loader.load("./vg-module-vector-collision-detector.so");
  }
};

const auto sample_rate = 1;

TEST_F(CollisionDetectorTest, TestNoCollision)
{
  auto& cd = add("vector/collision-detector");

  auto subj_data = vector<Frame>(1);
  auto& subj = subj_data[0];
  subj.points.push_back(Point(0, 0));  // Closed square
  subj.points.push_back(Point(1, 0, Colour::white));
  subj.points.push_back(Point(1, 1, Colour::white));
  subj.points.push_back(Point(0, 1, Colour::white));
  subj.points.push_back(Point(0, 0, Colour::white));

  auto& subjs = add_source(subj_data);
  subjs.connect("output", cd, "subject");

  auto obj_data = vector<Frame>(1);
  auto& obj = obj_data[0];
  obj.points.push_back(Point(2, 0));  // Closed square
  obj.points.push_back(Point(3, 0, Colour::white));
  obj.points.push_back(Point(3, 1, Colour::white));
  obj.points.push_back(Point(2, 1, Colour::white));
  obj.points.push_back(Point(2, 0, Colour::white));

  auto& objs = add_source(obj_data);
  objs.connect("output", cd, "object");

  auto collided = vector<Trigger>{};
  auto& snk = add_sink(collided, sample_rate);
  cd.connect("collided", snk, "input");

  run(2);  // test is 1 tick behind

  ASSERT_EQ(2, collided.size());
  ASSERT_FALSE(collided[0]);
  ASSERT_FALSE(collided[1]);
}

TEST_F(CollisionDetectorTest, TestCollision)
{
  auto& cd = add("vector/collision-detector");

  auto subj_data = vector<Frame>(1);
  auto& subj = subj_data[0];
  subj.points.push_back(Point(0, 0));  // Closed square
  subj.points.push_back(Point(1, 0, Colour::white));
  subj.points.push_back(Point(1, 1, Colour::white));
  subj.points.push_back(Point(0, 1, Colour::white));
  subj.points.push_back(Point(0, 0, Colour::white));

  auto& subjs = add_source(subj_data);
  subjs.connect("output", cd, "subject");

  auto obj_data = vector<Frame>(1);
  auto& obj = obj_data[0];
  obj.points.push_back(Point(0, 0));  // Closed square
  obj.points.push_back(Point(3, 0, Colour::white));
  obj.points.push_back(Point(3, 3, Colour::white));
  obj.points.push_back(Point(0, 3, Colour::white));
  obj.points.push_back(Point(0, 0, Colour::white));

  auto& objs = add_source(obj_data);
  objs.connect("output", cd, "object");

  auto collided = vector<Trigger>{};
  auto& snk = add_sink(collided, sample_rate);
  cd.connect("collided", snk, "input");

  run(2);  // test is 1 tick behind

  ASSERT_EQ(2, collided.size());
  ASSERT_FALSE(collided[0]);
  ASSERT_TRUE(collided[1]);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
