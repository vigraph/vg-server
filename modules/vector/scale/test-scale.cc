//==========================================================================
// ViGraph dataflow module: vector/scale/test-scale.cc
//
// Tests for <scale> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include "../../module-test.h"

ModuleLoader loader;

const auto sample_rate = 1;

TEST(ScaleTest, TestDefaultScaleHasNoEffect)
{
  GraphTester<Frame> tester{loader, sample_rate};

  auto& trans = tester.add("vector/scale");

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  for(auto i=0u; i<50; i++)
    fr.points.push_back(Point(i, i*2, i*3, Colour::white));

  auto& frs = tester.add_source(fr_data);
  frs.connect("output", trans, "input");

  tester.capture_from(trans, "output");

  tester.run();

  const auto frames = tester.get_output();

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

TEST(ScaleTest, TestDefaultScaleXYZ)
{
  GraphTester<Frame> tester{loader, sample_rate};

  auto& trans = tester.add("vector/scale")
                      .set("x", 1.0)
                      .set("y", 2.0)
                      .set("z", 3.0);

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  for(auto i=0u; i<50; i++)
    fr.points.push_back(Point(i, i*2, i*3, Colour::red));

  auto& frs = tester.add_source(fr_data);
  frs.connect("output", trans, "input");

  tester.capture_from(trans, "output");

  tester.run();

  const auto frames = tester.get_output();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];
  ASSERT_EQ(50, frame.points.size());
  for(auto i=0u; i<frame.points.size(); i++)
  {
    const auto& p = frame.points[i];
    EXPECT_EQ(i  *1.0, p.x);
    EXPECT_EQ(i*2*2.0, p.y);
    EXPECT_EQ(i*3*3.0, p.z);
    EXPECT_EQ(Colour::red, p.c);
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  loader.load("./vg-module-vector-scale.so");
  return RUN_ALL_TESTS();
}
