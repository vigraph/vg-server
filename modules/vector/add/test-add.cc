//==========================================================================
// ViGraph dataflow module: vector/add/test-add.cc
//
// Tests for <add> filter
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include "../../module-test.h"

class AddTest: public GraphTester
{
public:
  AddTest()
  {
    loader.load("./vg-module-vector-add.so");
  }
};

const auto sample_rate = 1;

TEST_F(AddTest, TestAddition)
{
  auto& vadd = add("vector/add");

  auto fri_data = vector<Frame>(1);
  auto& fri = fri_data[0];
  for(auto i=0u; i<50; i++)
    fri.points.push_back(Point(i, i*2, i*3, Colour::white));

  auto& fris = add_source(fri_data);
  fris.connect("output", vadd, "input");

  auto fro_data = vector<Frame>(1);
  auto& fro = fro_data[0];
  for(auto i=0u; i<10; i++)
    fro.points.push_back(Point(1, 2, 3, Colour::white));

  auto& fros = add_source(fro_data);
  fros.connect("output", vadd, "offset");

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  vadd.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];
  ASSERT_EQ(50, frame.points.size());
  for(auto i=0u; i<frame.points.size(); i++)
  {
    const auto& p = frame.points[i];
    if (i<10)
    {
      // Combined
      EXPECT_EQ(i+1,   p.x);
      EXPECT_EQ(i*2+2, p.y);
      EXPECT_EQ(i*3+3, p.z);
    }
    else
    {
      // Original
      EXPECT_EQ(i,   p.x);
      EXPECT_EQ(i*2, p.y);
      EXPECT_EQ(i*3, p.z);
    }
    EXPECT_EQ(Colour::white, p.c);
  }
}

TEST_F(AddTest, TestAdditionSizesReversed)
{
  auto& vadd = add("vector/add");

  auto fri_data = vector<Frame>(1);
  auto& fri = fri_data[0];
  for(auto i=0u; i<10; i++)
    fri.points.push_back(Point(i, i*2, i*3, Colour::white));

  auto& fris = add_source(fri_data);
  fris.connect("output", vadd, "input");

  auto fro_data = vector<Frame>(1);
  auto& fro = fro_data[0];
  for(auto i=0u; i<50; i++)
    fro.points.push_back(Point(1, 2, 3, Colour::white));

  auto& fros = add_source(fro_data);
  fros.connect("output", vadd, "offset");

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  vadd.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];
  ASSERT_EQ(50, frame.points.size());
  for(auto i=0u; i<frame.points.size(); i++)
  {
    const auto& p = frame.points[i];
    if (i<10)
    {
      // Combined
      EXPECT_EQ(i+1,   p.x);
      EXPECT_EQ(i*2+2, p.y);
      EXPECT_EQ(i*3+3, p.z);
    }
    else
    {
      // Offset only
      EXPECT_EQ(1, p.x);
      EXPECT_EQ(2, p.y);
      EXPECT_EQ(3, p.z);
    }
    EXPECT_EQ(Colour::white, p.c);
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
