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

TEST(ArtNetPacketTest, TestBasePacketWrite)
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

TEST(ArtNetPacketTest, TestBasePacketRead)
{
  string data("Art-Net\x00"
              "\x00\x50"   // OpDmx
              "\x00\x0e",  // Version 14
              12);

  Packet packet;
  istringstream iss(data);
  Channel::StreamReader reader(iss);
  ASSERT_NO_THROW(packet.read(reader));
  EXPECT_EQ(op_dmx, packet.opcode);
}

TEST(ArtNetPacketTest, TestDMXPacketWrite)
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
  EXPECT_EQ(22, packet.length());

  ostringstream oss;
  Channel::StreamWriter writer(oss);
  ASSERT_NO_THROW(packet.write(writer));
  EXPECT_EQ(data, oss.str());
}

TEST(ArtNetPacketTest, TestDMXPacketRead)
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

  DMXPacket packet;
  istringstream iss(data);
  Channel::StreamReader reader(iss);
  ASSERT_NO_THROW(packet.read(reader));
  EXPECT_EQ(op_dmx, packet.opcode);
  EXPECT_EQ(42, packet.sequence);
  EXPECT_EQ(0x123, packet.port_address);
  ASSERT_EQ(4, packet.data.size());
  EXPECT_EQ(1, packet.data[0]);
  EXPECT_EQ(0x55, packet.data[1]);
  EXPECT_EQ(0xAA, packet.data[2]);
  EXPECT_EQ(0xFF, packet.data[3]);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
