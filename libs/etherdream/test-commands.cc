//==========================================================================
// ViGraph Ether Dream protocol library: test-commands.cc
//
// Tests for interface command format
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
using namespace ViGraph::Geometry;
using namespace ObTools;
using namespace std;

// Test channel
class TestChannel: public DataChannel
{
private:
  void send(const vector<uint8_t>& data)
  {
    received_data.insert(received_data.end(), data.begin(), data.end());
  }

public:
  vector<uint8_t> received_data;
};

TEST(CommandsTest, test_prepare)
{
  TestChannel channel;
  CommandSender commands(channel);
  commands.prepare();
  ASSERT_EQ(1, channel.received_data.size());
  EXPECT_EQ('p', channel.received_data[0]);
}

TEST(CommandsTest, test_begin_playback)
{
  TestChannel channel;
  CommandSender commands(channel);
  commands.begin_playback(0x01020304ul);

  vector<uint8_t> expected
  {
    'b',
    0, 0,
    4, 3, 2, 1
  };

  ASSERT_EQ(7, channel.received_data.size());
  EXPECT_EQ(expected, channel.received_data);
}

TEST(CommandsTest, test_queue_rate_change)
{
  TestChannel channel;
  CommandSender commands(channel);
  commands.queue_rate_change(0x0A0B0C0Dul);

  vector<uint8_t> expected
  {
    'q',
    0x0D, 0x0C, 0x0B, 0x0A
  };

  ASSERT_EQ(5, channel.received_data.size());
  EXPECT_EQ(expected, channel.received_data);
}

TEST(CommandsTest, test_sending_blank_data)
{
  TestChannel channel;
  CommandSender commands(channel);
  vector<Point> points;
  commands.send(points);

  ASSERT_EQ(3, channel.received_data.size());
  EXPECT_EQ('d', channel.received_data[0]);
  EXPECT_EQ(0, channel.received_data[1]);
  EXPECT_EQ(0, channel.received_data[2]);
}

TEST(CommandsTest, test_sending_centre_point_white)
{
  TestChannel channel;
  CommandSender commands(channel);
  vector<Point> points;
  points.push_back(Point(0,0,Colour::white));
  commands.send(points);

  vector<uint8_t> expected
  {
    'd', 1, 0,
    0,0, // control
    0,0, // x
    0,0, // y
    0xff, 0xff, // r
    0xff, 0xff, // g
    0xff, 0xff, // b
    0, 0, // i
    0, 0, // u1
    0, 0  // u2
  };

  ASSERT_EQ(21, channel.received_data.size());
  EXPECT_EQ(expected, channel.received_data);
}

TEST(CommandsTest, test_sending_bottom_left_red)
{
  TestChannel channel;
  CommandSender commands(channel);
  vector<Point> points;
  points.push_back(Point(-0.5,-0.5,Colour::red));
  commands.send(points);

  vector<uint8_t> expected
  {
    'd', 1, 0,
    0,0, // control
    1,0x80, // x - note 0x8001 = -32767 - can't reach full negative range
    1,0x80, // y
    0xff, 0xff, // r
    0, 0, // g
    0, 0, // b
    0, 0, // i
    0, 0, // u1
    0, 0  // u2
  };

  ASSERT_EQ(21, channel.received_data.size());
  EXPECT_EQ(expected, channel.received_data);
}

TEST(CommandsTest, test_sending_top_left_rgb)
{
  TestChannel channel;
  CommandSender commands(channel);
  vector<Point> points;
  points.push_back(Point(0.5,0.5,Colour::RGB(0,0.5,1)));
  commands.send(points);

  vector<uint8_t> expected
  {
    'd', 1, 0,
    0,0, // control
    0xff,0x7f, // x - 0x7fff
    0xff,0x7f, // y - 0x7fff
    0,0,       // r
    0xff,0x7f, // g - mid range = 0x7fff
    0xff,0xff, // b
    0, 0, // i
    0, 0, // u1
    0, 0  // u2
  };

  ASSERT_EQ(21, channel.received_data.size());
  EXPECT_EQ(expected, channel.received_data);
}

TEST(CommandsTest, test_sending_two_points_with_rate_change)
{
  TestChannel channel;
  CommandSender commands(channel);
  vector<Point> points;
  points.push_back(Point(0,0,Colour::black));
  points.push_back(Point(0,0,Colour::white));
  commands.send(points, true);

  vector<uint8_t> expected
  {
    'd', 2, 0,
    0,0x80, // control with rate change
    0,0, // x
    0,0, // y
    0, 0, // r
    0, 0, // g
    0, 0, // b
    0, 0, // i
    0, 0, // u1
    0, 0,  // u2

    0,0, // control, no rate change
    0,0, // x
    0,0, // y
    0xff, 0xff, // r
    0xff, 0xff, // g
    0xff, 0xff, // b
    0, 0, // i
    0, 0, // u1
    0, 0  // u2
  };

  ASSERT_EQ(39, channel.received_data.size());
  EXPECT_EQ(expected, channel.received_data);
}

TEST(CommandsTest, test_stop)
{
  TestChannel channel;
  CommandSender commands(channel);
  commands.stop_playback();
  ASSERT_EQ(1, channel.received_data.size());
  EXPECT_EQ('s', channel.received_data[0]);
}

TEST(CommandsTest, test_e_stop)
{
  TestChannel channel;
  CommandSender commands(channel);
  commands.emergency_stop();
  ASSERT_EQ(1, channel.received_data.size());
  EXPECT_EQ(0, channel.received_data[0]);
}

TEST(CommandsTest, test_clear_e_stop)
{
  TestChannel channel;
  CommandSender commands(channel);
  commands.clear_emergency_stop();
  ASSERT_EQ(1, channel.received_data.size());
  EXPECT_EQ('c', channel.received_data[0]);
}

TEST(CommandsTest, test_ping)
{
  TestChannel channel;
  CommandSender commands(channel);
  commands.ping();
  ASSERT_EQ(1, channel.received_data.size());
  EXPECT_EQ('?', channel.received_data[0]);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
