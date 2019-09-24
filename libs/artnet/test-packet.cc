//==========================================================================
// ViGraph Art-Net protocol library: test-packet.cc
//
// Tests for base packet writing
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-artnet.h"
#include <gtest/gtest.h>
#include <sstream>

namespace {

using namespace ViGraph;
using namespace ViGraph::ArtNet;

TEST(ArtNetPacketWriterTest, TestBasePacket)
{
  string data("Art-Net\x00"
              "\x00\x50"   // OpDmx
              "\x00\x0e",  // Version 14
              12);

  Packet packet(OpCode::op_dmx);
  ostringstream oss;
  Channel::StreamWriter writer(oss);
  ASSERT_NO_THROW(packet.write(writer));
  EXPECT_EQ(data, oss.str());
}

TEST(ArtNetPacketWriterTest, TestBaseDMXPacket)
{
  string data("Art-Net\x00"
              "\x00\x50"   // OpDmx
              "\x00\x0e"   // Version 14
              "\x2a"       // Sequence 42
              "\x00"       // Physical 0
              "\x23\x01"   // Net 1, Subnet 2, universe 3
              "\x00\x04"   // Length
              "\x01\x55\xaa\xff",  // DMX
              22);

  DMXPacket packet(42, 0x123);
  packet.data.push_back(0x01);
  packet.data.push_back(0x55);
  packet.data.push_back(0xaa);
  packet.data.push_back(0xff);

  ostringstream oss;
  Channel::StreamWriter writer(oss);
  ASSERT_NO_THROW(packet.write(writer));
  EXPECT_EQ(data, oss.str());
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
