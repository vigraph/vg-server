//==========================================================================
// Tests for <pattern> module
//
// Copyright (c) 2017-2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include "../../module-test.h"

class PatternTest: public GraphTester
{
public:
  PatternTest()
  {
    loader.load("./vg-module-vector-pattern.so");
  }
};

const auto sample_rate = 1;

TEST_F(PatternTest, TestDefaultHasNoEffect)
{
  auto& pat = add("vector/pattern");

  auto fr_data = vector<Frame>(1);
  fr_data.front().points.resize(10, Point{Vector{}, Colour::RGB{1}});
  auto& src = add_source(fr_data);
  src.connect("output", pat, "input");

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  pat.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames.front();

  // Should be 10 points at (0,0), all white
  ASSERT_EQ(10, frame.points.size());
  for (const auto& p: frame.points)
  {
    EXPECT_EQ(0.0, p.x);
    EXPECT_EQ(0.0, p.y);
    EXPECT_EQ(0.0, p.z);
    EXPECT_EQ(1.0, p.c.r);
    EXPECT_EQ(1.0, p.c.g);
    EXPECT_EQ(1.0, p.c.b);
  }
}

TEST_F(PatternTest, TestSimpleAlternatingOneRepeat)
{
  auto& pat = add("vector/pattern")
              .set("colours", Integer{2});
  setup(pat);
  pat.set("colour1", Colour::RGB{1, 0, 0});
  pat.set("colour2", Colour::RGB{0, 1, 0});

  auto fr_data = vector<Frame>(1);
  fr_data.front().points.resize(10, Point{Vector{}, Colour::RGB{1}});
  auto& src = add_source(fr_data);
  src.connect("output", pat, "input");

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  pat.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames.front();

  // Should be 10 points at (0,0), 5 red, 5 green
  ASSERT_EQ(10, frame.points.size());
  for(auto i=0; i<5; i++)
  {
    const auto&p = frame.points[i];
    EXPECT_EQ(1.0, p.c.r) << i;
    EXPECT_EQ(0.0, p.c.g) << i;
    EXPECT_EQ(0.0, p.c.b) << i;
  }
  for(auto i=5; i<10; i++)
  {
    const auto&p = frame.points[i];
    EXPECT_EQ(0.0, p.c.r) << i;
    EXPECT_EQ(1.0, p.c.g) << i;
    EXPECT_EQ(0.0, p.c.b) << i;
  }
}

TEST_F(PatternTest, TestSimpleAlternating5Repeats)
{
  auto& pat = add("vector/pattern")
              .set("colours", Integer{2});
  setup(pat);
  pat.set("colour1", Colour::RGB{1, 1, 0})
     .set("colour2", Colour::RGB{0, 0, 1})
     .set("repeats", 5.0);

  auto fr_data = vector<Frame>(1);
  fr_data.front().points.resize(10, Point{Vector{}, Colour::RGB{1}});
  auto& src = add_source(fr_data);
  src.connect("output", pat, "input");

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  pat.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames.front();

  // Should be 10 points at (0,0), alternating yellow, blue
  ASSERT_EQ(10, frame.points.size());
  for(auto i=0; i<10; i++)
  {
    const auto&p = frame.points[i];
    if (i&1)
    {
      EXPECT_EQ(0.0, p.c.r) << i;
      EXPECT_EQ(0.0, p.c.g) << i;
      EXPECT_EQ(1.0, p.c.b) << i;
    }
    else
    {
      EXPECT_EQ(1.0, p.c.r) << i;
      EXPECT_EQ(1.0, p.c.g) << i;
      EXPECT_EQ(0.0, p.c.b) << i;
    }
  }
}

TEST_F(PatternTest, TestSimpleAlternating5RepeatsAntiPhase)
{
  auto& pat = add("vector/pattern")
              .set("colours", Integer{2});
  setup(pat);
  pat.set("colour1", Colour::RGB{1, 1, 0})
     .set("colour2", Colour::RGB{0, 0, 1})
     .set("repeats", 5.0)
     .set("phase", 0.5);

  auto fr_data = vector<Frame>(1);
  fr_data.front().points.resize(10, Point{Vector{}, Colour::RGB{1}});
  auto& src = add_source(fr_data);
  src.connect("output", pat, "input");

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  pat.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames.front();

  // Should be 10 points at (0,0), alternating blue, yellow
  ASSERT_EQ(10, frame.points.size());
  for(auto i=0; i<10; i++)
  {
    const auto&p = frame.points[i];
    if (i&1)
    {
      EXPECT_EQ(1.0, p.c.r) << i;
      EXPECT_EQ(1.0, p.c.g) << i;
      EXPECT_EQ(0.0, p.c.b) << i;
    }
    else
    {
      EXPECT_EQ(0.0, p.c.r) << i;
      EXPECT_EQ(0.0, p.c.g) << i;
      EXPECT_EQ(1.0, p.c.b) << i;
    }
  }
}

TEST_F(PatternTest, TestBlendOneRepeat)
{
  auto& pat = add("vector/pattern")
              .set("colours", Integer{2})
              .set("blend", true);
  setup(pat);
  pat.set("colour1", Colour::RGB{1, 0, 0})
     .set("colour2", Colour::RGB{0, 1, 0});

  auto fr_data = vector<Frame>(1);
  fr_data.front().points.resize(10, Point{Vector{}, Colour::RGB{1}});
  auto& src = add_source(fr_data);
  src.connect("output", pat, "input");

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  pat.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames.front();

  // Should be 10 points at (0,0), alternating blue, yellow
  ASSERT_EQ(10, frame.points.size());
  for(auto i=0; i<5; i++)
  {
    const auto&p = frame.points[i];
    EXPECT_DOUBLE_EQ(1-i/5.0, p.c.r) << i;
    EXPECT_DOUBLE_EQ(i/5.0, p.c.g) << i;
    EXPECT_DOUBLE_EQ(0.0, p.c.b) << i;
  }
  for(auto i=5; i<10; i++)
  {
    const auto&p = frame.points[i];
    EXPECT_DOUBLE_EQ((i-5)/5.0, p.c.r) << i;
    EXPECT_DOUBLE_EQ(1-(i-5)/5.0, p.c.g) << i;
    EXPECT_DOUBLE_EQ(0.0, p.c.b) << i;
  }
}

TEST_F(PatternTest, TestProportionalAlternatingOneRepeat)
{
  auto& pat = add("vector/pattern")
              .set("colours", Integer{2});
  setup(pat);
  pat.set("colour1", Colour::RGB{1, 0, 0})
     .set("proportion1", Number{3})
     .set("colour2", Colour::RGB{0, 1, 0})
     .set("proportion2", Number{7});

  auto fr_data = vector<Frame>(1);
  fr_data.front().points.resize(10, Point{Vector{}, Colour::RGB{1}});
  auto& src = add_source(fr_data);
  src.connect("output", pat, "input");

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  pat.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames.front();

  // Should be 10 points at (0,0), 3 red, 7 green
  ASSERT_EQ(10, frame.points.size());
  for(auto i=0; i<3; i++)
  {
    const auto&p = frame.points[i];
    EXPECT_EQ(1.0, p.c.r) << i;
    EXPECT_EQ(0.0, p.c.g) << i;
    EXPECT_EQ(0.0, p.c.b) << i;
  }
  for(auto i=3; i<10; i++)
  {
    const auto&p = frame.points[i];
    EXPECT_EQ(0.0, p.c.r) << i;
    EXPECT_EQ(1.0, p.c.g) << i;
    EXPECT_EQ(0.0, p.c.b) << i;
  }
}

TEST_F(PatternTest, TestProportionalBlendOneRepeat)
{
  auto& pat = add("vector/pattern")
              .set("colours", Integer{2})
              .set("blend", true);
  setup(pat);
  pat.set("colour1", Colour::RGB{1, 0, 0})
     .set("proportion1", Number{3})
     .set("colour2", Colour::RGB{0, 1, 0})
     .set("proportion2", Number{7});

  auto fr_data = vector<Frame>(1);
  fr_data.front().points.resize(10, Point{Vector{}, Colour::RGB{1}});
  auto& src = add_source(fr_data);
  src.connect("output", pat, "input");

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  pat.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames.front();

  // Should be 10 points at (0,0), alternating blue, yellow
  ASSERT_EQ(10, frame.points.size());
  for(auto i=0; i<3; i++)
  {
    const auto&p = frame.points[i];
    EXPECT_DOUBLE_EQ(1-i/3.0, p.c.r) << i;
    EXPECT_DOUBLE_EQ(i/3.0, p.c.g) << i;
    EXPECT_DOUBLE_EQ(0.0, p.c.b) << i;
  }
  for(auto i=3; i<10; i++)
  {
    const auto&p = frame.points[i];
    EXPECT_DOUBLE_EQ((i-3)/7.0, p.c.r) << i;
    EXPECT_DOUBLE_EQ(1-(i-3)/7.0, p.c.g) << i;
    EXPECT_DOUBLE_EQ(0.0, p.c.b) << i;
  }
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
