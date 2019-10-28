//==========================================================================
// ViGraph dataflow module: vector/svg/test-svg.cc
//
// Tests for <svg> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include "../../module-test.h"

class SVGTest: public GraphTester
{
public:
  SVGTest()
  {
    loader.load("./vg-module-vector-svg.so");
  }
};

const auto sample_rate = 1;

TEST_F(SVGTest, TestSimpleLineNotNormalised)
{
  auto& svg = add("vector/svg")
              .set("path", string("M 0 0 L 2 1"))
              .set("normalise", false);

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  svg.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];
  ASSERT_EQ(2, frame.points.size());

  const auto& p0 = frame.points[0];
  EXPECT_EQ(0, p0.x);
  EXPECT_EQ(0, p0.y);
  EXPECT_EQ(0, p0.z);
  EXPECT_TRUE(p0.is_blanked());

  const auto& p1 = frame.points[1];
  EXPECT_EQ(2, p1.x);
  EXPECT_EQ(1, p1.y);  // Note not flipped
  EXPECT_EQ(0, p1.z);
  EXPECT_TRUE(p1.is_lit());
}

TEST_F(SVGTest, TestSimpleLineNormalised)
{
  auto& svg = add("vector/svg")
              .set("path", string("M 0 0 L 2 1"));

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  svg.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];
  ASSERT_EQ(2, frame.points.size());

  const auto& p0 = frame.points[0];
  EXPECT_EQ(-0.5, p0.x);
  EXPECT_EQ(0.25, p0.y);  // Note centred then flipped
  EXPECT_EQ(0, p0.z);
  EXPECT_TRUE(p0.is_blanked());

  const auto& p1 = frame.points[1];
  EXPECT_EQ(0.5, p1.x);
  EXPECT_EQ(-0.25, p1.y);
  EXPECT_EQ(0, p1.z);
  EXPECT_TRUE(p1.is_lit());
}

TEST_F(SVGTest, TestSmoothQuadraticWithDefaultPrecision)
{
  auto& svg = add("vector/svg")
              .set("path", string("M 0 0 T 2 1"));

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  svg.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];
  ASSERT_EQ(11, frame.points.size());
}

TEST_F(SVGTest, TestSmoothQuadraticWithReducedPrecision)
{
  auto& svg = add("vector/svg")
              .set("path", string("M 0 0 T 2 1"))
              .set("precision", 0.2);

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  svg.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];
  ASSERT_EQ(6, frame.points.size());
}

TEST_F(SVGTest, TestReadFromFile)
{
  char templ[10] = "SVGXXXXXX";
  auto fd = mkstemp(templ);
  ASSERT_NE(-1, fd);
  File::Path tempfile(templ);
  tempfile.write_all("<svg><path d='M 0 0 L 2 1'/></svg>");

  auto& svg = add("vector/svg")
              .set("file", tempfile.str());

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  svg.connect("output", snk, "input");

  run();
  close(fd);
  tempfile.erase();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];
  ASSERT_EQ(2, frame.points.size());
}

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    Log::logger.connect(chan_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
