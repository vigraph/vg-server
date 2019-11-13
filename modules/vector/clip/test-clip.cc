//==========================================================================
// ViGraph dataflow module: vector/clip/test-clip.cc
//
// Tests for <clip> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include "../../module-test.h"

class ClipTest: public GraphTester
{
public:
  ClipTest()
  {
    loader.load("./vg-module-vector-clip.so");
  }
};

const auto sample_rate = 1;

TEST_F(ClipTest, TestDefaultClipNoChangeInside)
{
  auto& clip = add("vector/clip");

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  for(auto i=0u; i<100; i++)
    fr.points.push_back(Point(i/100-0.5,i/100-0.5,i/100-0.5, Colour::white));

  auto& frs = add_source(fr_data);
  frs.connect("output", clip, "input");

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  clip.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];
  ASSERT_EQ(100, frame.points.size());
  for(auto i=0u; i<frame.points.size(); i++)
  {
    const auto& p = frame.points[i];
    EXPECT_EQ(i/100-0.5, p.x);
    EXPECT_EQ(i/100-0.5, p.y);
    EXPECT_EQ(i/100-0.5, p.z);
    EXPECT_EQ(Colour::white, p.c);
  }
}

TEST_F(ClipTest, TestDefaultClipOutsideWithAlpha)
{
  auto& clip = add("vector/clip").set("alpha", 0.5);

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point(0,0));
  fr.points.push_back(Point(2,0, Colour::white));

  auto& frs = add_source(fr_data);
  frs.connect("output", clip, "input");

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  clip.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];
  ASSERT_EQ(2, frame.points.size());
  EXPECT_DOUBLE_EQ(0, frame.points[0].x);
  EXPECT_TRUE(frame.points[0].is_blanked());
  EXPECT_DOUBLE_EQ(2.0, frame.points[1].x);
  EXPECT_FALSE(frame.points[1].is_blanked());
  EXPECT_DOUBLE_EQ(0.5, frame.points[1].c.get_intensity());
}

TEST_F(ClipTest, TestDefaultClipOutsideMovesToLastAndBlanks)
{
  auto& clip = add("vector/clip");

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point(0,0));
  fr.points.push_back(Point(2,0, Colour::white));

  auto& frs = add_source(fr_data);
  frs.connect("output", clip, "input");

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  clip.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];
  ASSERT_EQ(2, frame.points.size());
  EXPECT_DOUBLE_EQ(0, frame.points[0].x);
  EXPECT_TRUE(frame.points[0].is_blanked());
  EXPECT_DOUBLE_EQ(0, frame.points[1].x);
  EXPECT_TRUE(frame.points[1].is_blanked());
}

TEST_F(ClipTest, TestDefaultClipInsideMovesToLastAndBlanks)
{
  auto& clip = add("vector/clip")
    .set("exclude", true)
    .set("min-x", 0.5)
    .set("max-x", 1.5);

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point(0,0));
  fr.points.push_back(Point(1,0, Colour::white));

  auto& frs = add_source(fr_data);
  frs.connect("output", clip, "input");

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  clip.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];
  ASSERT_EQ(2, frame.points.size());
  EXPECT_DOUBLE_EQ(0, frame.points[0].x);
  EXPECT_TRUE(frame.points[0].is_blanked());
  EXPECT_DOUBLE_EQ(0, frame.points[1].x);
  EXPECT_TRUE(frame.points[1].is_blanked());
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
