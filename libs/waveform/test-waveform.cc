//==========================================================================
// ViGraph Waveform library: test-waveform.cc
//
// Tests for Waveform functions
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-waveform.h"
#include <gtest/gtest.h>

namespace {

using namespace ViGraph;
using namespace ViGraph::Waveform;

TEST(WaveformTest, TestNone)
{
  EXPECT_EQ(0.0, get_value(Type::none, 0.0, 0.0));
  EXPECT_EQ(0.0, get_value(Type::none, 0.5, 0.0));
  EXPECT_EQ(0.0, get_value(Type::none, 0.5, 0.2));
}

TEST(WaveformTest, TestSaw)
{
  EXPECT_EQ(0.0, get_value(Type::saw, 0.0, 0.0));
  EXPECT_EQ(0.25, get_value(Type::saw, 0.5, 0.125));
  EXPECT_EQ(0.5, get_value(Type::saw, 0.5, 0.25));
  EXPECT_EQ(0.75, get_value(Type::saw, 0.5, 0.375));
  EXPECT_NEAR(1.0, get_value(Type::saw, 0.0, 0.499999), 0.0001);
  EXPECT_EQ(-1.0, get_value(Type::saw, 0.0, 0.5));
  EXPECT_EQ(-0.75, get_value(Type::saw, 0.5, 0.625));
  EXPECT_EQ(-0.5, get_value(Type::saw, 0.5, 0.75));
  EXPECT_EQ(-0.25, get_value(Type::saw, 0.5, 0.875));
  EXPECT_EQ(0.0, get_value(Type::saw, 0.0, 1.0));
}

TEST(WaveformTest, TestSquare)
{
  EXPECT_EQ(1.0, get_value(Type::square, 0.5, 0.0));
  EXPECT_EQ(1.0, get_value(Type::square, 0.5, 0.125));
  EXPECT_EQ(1.0, get_value(Type::square, 0.5, 0.25));
  EXPECT_EQ(1.0, get_value(Type::square, 0.5, 0.375));
  EXPECT_EQ(-1.0, get_value(Type::square, 0.5, 0.5));
  EXPECT_EQ(-1.0, get_value(Type::square, 0.5, 0.625));
  EXPECT_EQ(-1.0, get_value(Type::square, 0.5, 0.75));
  EXPECT_EQ(-1.0, get_value(Type::square, 0.5, 0.875));
  EXPECT_EQ(-1.0, get_value(Type::square, 0.5, 0.9999));
}

TEST(WaveformTest, TestTriangle)
{
  EXPECT_EQ(0.0, get_value(Type::triangle, 0.5, 0.0));
  EXPECT_EQ(0.5, get_value(Type::triangle, 0.5, 0.125));
  EXPECT_EQ(1.0, get_value(Type::triangle, 0.5, 0.25));
  EXPECT_EQ(0.5, get_value(Type::triangle, 0.5, 0.375));
  EXPECT_EQ(0.0, get_value(Type::triangle, 0.5, 0.5));
  EXPECT_EQ(-0.5, get_value(Type::triangle, 0.5, 0.625));
  EXPECT_EQ(-1.0, get_value(Type::triangle, 0.5, 0.75));
  EXPECT_EQ(-0.5, get_value(Type::triangle, 0.5, 0.875));
}

TEST(WaveformTest, TestSin)
{
  EXPECT_NEAR(0.0, get_value(Type::sin, 0.5, 0.0), 0.0001);
  EXPECT_NEAR(0.7071, get_value(Type::sin, 0.5, 0.125), 0.0001);
  EXPECT_NEAR(1.0, get_value(Type::sin, 0.5, 0.25), 0.0001);
  EXPECT_NEAR(0.7071, get_value(Type::sin, 0.5, 0.375), 0.0001);
  EXPECT_NEAR(0.0, get_value(Type::sin, 0.5, 0.5), 0.0001);
  EXPECT_NEAR(-0.7071, get_value(Type::sin, 0.5, 0.625), 0.0001);
  EXPECT_NEAR(-1.0, get_value(Type::sin, 0.5, 0.75), 0.0001);
  EXPECT_NEAR(-0.7071, get_value(Type::sin, 0.5, 0.875), 0.0001);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
