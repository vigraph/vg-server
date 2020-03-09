//==========================================================================
// ViGraph dataflow module: vector/slice/test-slice.cc
//
// Tests for <slice> filter
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include "../../module-test.h"

class SliceTest: public GraphTester
{
public:
  SliceTest()
  {
    loader.load("./vg-module-vector-slice.so");
  }
};

const auto sample_rate = 1;

TEST_F(SliceTest, TestDefaultSliceHasNoEffect)
{
  auto& trans = add("vector/slice");

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point());  // Blanked
  for(auto i=1u; i<51; i++)
    fr.points.push_back(Point(i, i*2, i*3, Colour::white));

  auto& frs = add_source(fr_data);
  frs.connect("output", trans, "input");

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  trans.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];
  ASSERT_EQ(51, frame.points.size());
  EXPECT_EQ(Point(), frame.points[0]);
  EXPECT_TRUE(frame.points[0].is_blanked());
  for(auto i=1u; i<frame.points.size(); i++)
  {
    const auto& p = frame.points[i];
    EXPECT_EQ(i, p.x);
    EXPECT_EQ(i*2, p.y);
    EXPECT_EQ(i*3, p.z);
    EXPECT_EQ(Colour::white, p.c);
  }
}

TEST_F(SliceTest, TestChangingStartAndLengthRemovesPointsAtStart)
{
  auto& trans = add("vector/slice")
    .set("start", 0.5)
    .set("length", 0.5);

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point());  // Blanked
  for(auto i=1u; i<51; i++)
    fr.points.push_back(Point(i, i*2, i*3, Colour::white));

  auto& frs = add_source(fr_data);
  frs.connect("output", trans, "input");

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  trans.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];
  ASSERT_EQ(25, frame.points.size());
  EXPECT_EQ(Point(25, 50, 75), frame.points[0]);  // Blanked first point
  EXPECT_TRUE(frame.points[0].is_blanked());
  for(auto i=1u; i<frame.points.size(); i++)
  {
    const auto& p = frame.points[i];
    auto j=i+25;
    EXPECT_EQ(j, p.x);
    EXPECT_EQ(j*2, p.y);
    EXPECT_EQ(j*3, p.z);
    EXPECT_EQ(Colour::white, p.c);
  }
}

TEST_F(SliceTest, TestChangingLengthRemovesPointsAtEnd)
{
  auto& trans = add("vector/slice").set("length", 0.5);

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point());  // Blanked
  for(auto i=1u; i<51; i++)
    fr.points.push_back(Point(i, i*2, i*3, Colour::white));

  auto& frs = add_source(fr_data);
  frs.connect("output", trans, "input");

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  trans.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];
  ASSERT_EQ(25, frame.points.size());
  EXPECT_EQ(Point(), frame.points[0]);  // Blanked first point
  EXPECT_TRUE(frame.points[0].is_blanked());
  for(auto i=1u; i<frame.points.size(); i++)
  {
    const auto& p = frame.points[i];
    EXPECT_EQ(i, p.x);
    EXPECT_EQ(i*2, p.y);
    EXPECT_EQ(i*3, p.z);
    EXPECT_EQ(Colour::white, p.c);
  }
}

TEST_F(SliceTest, TestChangingStartOnlyRotates)
{
  auto& trans = add("vector/slice")
    .set("start", 0.5);

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point());  // Blanked
  for(auto i=1u; i<51; i++)
    fr.points.push_back(Point(i, i*2, i*3, Colour::white));

  auto& frs = add_source(fr_data);
  frs.connect("output", trans, "input");

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  trans.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];
  ASSERT_EQ(51, frame.points.size());
  EXPECT_EQ(Point(25, 50, 75), frame.points[0]);  // Blanked first point
  EXPECT_TRUE(frame.points[0].is_blanked());
  for(auto i=1u; i<frame.points.size(); i++)
  {
    const auto& p = frame.points[i];
    auto j=(i>25)?i-26:i+25;
    EXPECT_EQ(j, p.x);
    EXPECT_EQ(j*2, p.y);
    EXPECT_EQ(j*3, p.z);
    EXPECT_EQ(j?Colour::white:Colour::black, p.c);  // Original blanked
  }
}

TEST_F(SliceTest, TestSillyValuesDontBreak)
{
  auto& trans = add("vector/slice")
    .set("start", -1.0)
    .set("length", 2.0);

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point());  // Blanked
  for(auto i=1u; i<51; i++)
    fr.points.push_back(Point(i, i*2, i*3, Colour::white));

  auto& frs = add_source(fr_data);
  frs.connect("output", trans, "input");

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  trans.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];
  ASSERT_EQ(51, frame.points.size());
  EXPECT_EQ(Point(), frame.points[0]);  // Blanked first point
  EXPECT_TRUE(frame.points[0].is_blanked());
  for(auto i=1u; i<frame.points.size(); i++)
  {
    const auto& p = frame.points[i];
    EXPECT_EQ(i, p.x);
    EXPECT_EQ(i*2, p.y);
    EXPECT_EQ(i*3, p.z);
    EXPECT_EQ(Colour::white, p.c);
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
