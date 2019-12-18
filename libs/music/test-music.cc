//==========================================================================
// ViGraph Music library: test-music.cc
//
// Tests for Music functions
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-music.h"
#include <gtest/gtest.h>

namespace {

using namespace ViGraph;
using namespace ViGraph::Music;

TEST(MusicTest, TestNotes)
{
  EXPECT_NEAR(261.625, cv_to_frequency(0), 0.001);  // C4
  EXPECT_DOUBLE_EQ(440.0, cv_to_frequency(0.75));   // A4, concert pitch
  EXPECT_NEAR(523.251, cv_to_frequency(1), 0.001);  // C5
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
