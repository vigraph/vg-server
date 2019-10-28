//==========================================================================
// ViGraph dataflow module: vector/rotate/test-rotate.cc
//
// Tests for <rotate> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include "../../module-test.h"

class RotateTest: public GraphTester
{
public:
  RotateTest()
  {
    loader.load("./vg-module-vector-rotate.so");
  }
};

const auto sample_rate = 1;

TEST_F(RotateTest, TestDefaultRotateHasNoEffect)
{
  auto& trans = add("vector/rotate");

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  for(auto i=0u; i<50; i++)
    fr.points.push_back(Point(i, i*2, i*3, Colour::white));

  auto& frs = add_source(fr_data);
  frs.connect("output", trans, "input");

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  trans.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];
  ASSERT_EQ(50, frame.points.size());
  for(auto i=0u; i<frame.points.size(); i++)
  {
    const auto& p = frame.points[i];
    EXPECT_EQ(i, p.x);
    EXPECT_EQ(i*2, p.y);
    EXPECT_EQ(i*3, p.z);
    EXPECT_EQ(Colour::white, p.c);
  }
}

TEST_F(RotateTest, TestRotateX)
{
  auto& trans = add("vector/rotate")
                .set("x", 0.5);

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  for(auto i=0u; i<50; i++)
    fr.points.push_back(Point(i, i*2, i*3, Colour::red));

  auto& frs = add_source(fr_data);
  frs.connect("output", trans, "input");

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  trans.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];
  ASSERT_EQ(50, frame.points.size());
  for(auto i=0u; i<frame.points.size(); i++)
  {
    const auto& p = frame.points[i];
    EXPECT_DOUBLE_EQ(i,      p.x);
    EXPECT_DOUBLE_EQ(i*-2.0, p.y);
    EXPECT_DOUBLE_EQ(i*-3.0, p.z);
    EXPECT_EQ(Colour::red, p.c);
  }
}

TEST_F(RotateTest, TestRotateY)
{
  auto& trans = add("vector/rotate")
                .set("y", 0.5);

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  for(auto i=0u; i<50; i++)
    fr.points.push_back(Point(i, i*2, i*3, Colour::red));

  auto& frs = add_source(fr_data);
  frs.connect("output", trans, "input");

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  trans.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];
  ASSERT_EQ(50, frame.points.size());
  for(auto i=0u; i<frame.points.size(); i++)
  {
    const auto& p = frame.points[i];
    EXPECT_DOUBLE_EQ(i*-1.0, p.x);
    EXPECT_DOUBLE_EQ(i* 2.0, p.y);
    EXPECT_DOUBLE_EQ(i*-3.0, p.z);
    EXPECT_EQ(Colour::red, p.c);
  }
}

TEST_F(RotateTest, TestRotateZ)
{
  auto& trans = add("vector/rotate")
                .set("z", 0.5);

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  for(auto i=0u; i<50; i++)
    fr.points.push_back(Point(i, i*2, i*3, Colour::red));

  auto& frs = add_source(fr_data);
  frs.connect("output", trans, "input");

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  trans.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];
  ASSERT_EQ(50, frame.points.size());
  for(auto i=0u; i<frame.points.size(); i++)
  {
    const auto& p = frame.points[i];
    EXPECT_DOUBLE_EQ(i*-1.0, p.x);
    EXPECT_DOUBLE_EQ(i*-2.0, p.y);
    EXPECT_DOUBLE_EQ(i* 3.0, p.z);
    EXPECT_EQ(Colour::red, p.c);
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
