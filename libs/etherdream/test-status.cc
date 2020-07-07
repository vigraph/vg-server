//==========================================================================
// ViGraph Ether Dream protocol library: test-interface.cc
//
// Tests for status parser
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-etherdream.h"
#include "ot-log.h"

#include <gtest/gtest.h>
#include <sstream>

namespace {

using namespace ViGraph;
using namespace ViGraph::EtherDream;
using namespace ObTools;
using namespace std;

TEST(StatusTest, test_read_fails_on_empty_input)
{
  Status status;
  vector<uint8_t> data;
  ASSERT_FALSE(status.read(data));
}

TEST(StatusTest, test_read_fails_on_too_short_input)
{
  Status status;
  vector<uint8_t> data(19);
  ASSERT_FALSE(status.read(data));
}

TEST(StatusTest, test_read_ok_on_valid_input)
{
  Status status;
  vector<uint8_t> data{
    42,     // protocol
    1,      // light engine state: warmup
    2,      // playback state: playing
    1,      // source: sd-card
    5, 0,   // light engine flags: e-stop network+active
    3, 0,   // playback flags: shutter open, underflow
    0, 1,   // source flags = 256
    1, 4,   // buffer fullness = 1025
    0, 0, 1, 0,  // point rate = 63336
    0, 0, 0, 1}; // point count = 16M
  ASSERT_TRUE(status.read(data));
  EXPECT_EQ(0, data.size());

  EXPECT_EQ(42, status.protocol);
  EXPECT_EQ(Status::LightEngineState::warmup, status.light_engine_state);
  EXPECT_EQ(Status::PlaybackState::playing, status.playback_state);
  EXPECT_EQ(Status::Source::sd_card, status.source);
  EXPECT_EQ(Status::LightEngineFlags::e_stop_network
            + Status::LightEngineFlags::e_stop_active,
            status.light_engine_flags);
  EXPECT_EQ(Status::PlaybackFlags::shutter_open
            + Status::PlaybackFlags::underflow,
            status.playback_flags);
  EXPECT_EQ(256, status.source_flags);
  EXPECT_EQ(1025, status.buffer_fullness);
  EXPECT_EQ(0x10000, status.point_rate);
  EXPECT_EQ(0x1000000, status.point_count);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
