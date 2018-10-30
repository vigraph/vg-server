//==========================================================================
// ViGraph IDN stream library: test-message.cc
//
// Tests for IDN message structure
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-idn.h"
#include <gtest/gtest.h>
#include <sstream>

namespace {

using namespace ViGraph;
using namespace ViGraph::IDN;

TEST(IDNMessageTest, TestBasicConstructor)
{
  Message message(Message::ChunkType::laser_wave_samples);
  EXPECT_EQ(Message::ChunkType::laser_wave_samples, message.chunk_type);
}

TEST(IDNMessageTest, TestChannelConfigurationCheck)
{
  Message message(Message::ChunkType::laser_wave_samples);
  EXPECT_FALSE(message.has_channel_configuration());
  message.cclf = true;
  EXPECT_TRUE(message.has_channel_configuration());
  message.chunk_type = Message::ChunkType::laser_frame_samples_sequel;
  EXPECT_FALSE(message.has_channel_configuration());
}

TEST(IDNMessageTest, TestLastFragmentCheck)
{
  Message message(Message::ChunkType::laser_wave_samples);
  EXPECT_FALSE(message.is_last_fragment());
  message.cclf = true;
  EXPECT_FALSE(message.is_last_fragment());
  message.chunk_type = Message::ChunkType::laser_frame_samples_sequel;
  EXPECT_TRUE(message.is_last_fragment());
  message.cclf = false;
  EXPECT_FALSE(message.is_last_fragment());
}

TEST(IDNMessageTest, TestAddingConfiguration)
{
  Message message(Message::ChunkType::laser_wave_samples);
  message.add_configuration(Message::Config::ServiceMode::graphic_continuous);
  EXPECT_TRUE(message.has_channel_configuration());
  EXPECT_EQ(Message::Config::ServiceMode::graphic_continuous,
            message.config.service_mode);

  EXPECT_FALSE(message.get_routing());
  message.set_routing();
  EXPECT_TRUE(message.get_routing());

  EXPECT_FALSE(message.get_close());
  message.set_close();
  EXPECT_TRUE(message.get_close());
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
