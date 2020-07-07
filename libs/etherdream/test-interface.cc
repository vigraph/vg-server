//==========================================================================
// ViGraph Ether Dream protocol library: test-interface.cc
//
// Tests for interface protocol
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

// Test interface
class TestInterface: public Interface
{
private:
  void send_data(vector<uint8_t>& data)
  {
    received_data.insert(received_data.end(), data.begin(), data.end());
  }

public:
  vector<uint8_t> received_data;
};

TEST(InterfaceTest, test_default_status_is_ready_idle)
{
  TestInterface iface;
  const Status& status = iface.get_last_status();
  ASSERT_EQ(Status::LightEngineState::ready, status.light_engine_state);
  ASSERT_EQ(Status::PlaybackState::idle, status.playback_state);
}

TEST(InterfaceTest, test_it_sends_a_ping_on_startup)
{
  TestInterface iface;
  iface.start();
  ASSERT_EQ(1, iface.received_data.size());
  EXPECT_EQ('?', iface.received_data[0]);
}

TEST(InterfaceTest, test_prepare)
{
  TestInterface iface;
  iface.prepare();
  ASSERT_EQ(1, iface.received_data.size());
  EXPECT_EQ('p', iface.received_data[0]);
}

TEST(InterfaceTest, test_begin_playback)
{
  TestInterface iface;
  iface.begin_playback(0x01020304ul);

  vector<uint8_t> expected
  {
    'b',
    0, 0,
    4, 3, 2, 1
  };

  ASSERT_EQ(7, iface.received_data.size());
  EXPECT_EQ(expected, iface.received_data);
}

TEST(InterfaceTest, test_queue_rate_change)
{
  TestInterface iface;
  iface.queue_rate_change(0x0A0B0C0Dul);

  vector<uint8_t> expected
  {
    'q',
    0x0D, 0x0C, 0x0B, 0x0A
  };

  ASSERT_EQ(5, iface.received_data.size());
  EXPECT_EQ(expected, iface.received_data);
}

TEST(InterfaceTest, test_sending_blank_data)
{
  TestInterface iface;
  vector<Point> points;
  iface.send(points);

  ASSERT_EQ(3, iface.received_data.size());
  EXPECT_EQ('d', iface.received_data[0]);
  EXPECT_EQ(0, iface.received_data[1]);
  EXPECT_EQ(0, iface.received_data[2]);
}

TEST(InterfaceTest, test_sending_centre_point_white)
{
  TestInterface iface;
  vector<Point> points;
  points.push_back(Point(0,0,Colour::white));
  iface.send(points);

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

  ASSERT_EQ(21, iface.received_data.size());
  EXPECT_EQ(expected, iface.received_data);
}

TEST(InterfaceTest, test_sending_bottom_left_red)
{
  TestInterface iface;
  vector<Point> points;
  points.push_back(Point(-0.5,-0.5,Colour::red));
  iface.send(points);

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

  ASSERT_EQ(21, iface.received_data.size());
  EXPECT_EQ(expected, iface.received_data);
}

TEST(InterfaceTest, test_sending_top_left_rgb)
{
  TestInterface iface;
  vector<Point> points;
  points.push_back(Point(0.5,0.5,Colour::RGB(0,0.5,1)));
  iface.send(points);

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

  ASSERT_EQ(21, iface.received_data.size());
  EXPECT_EQ(expected, iface.received_data);
}

TEST(InterfaceTest, test_stop)
{
  TestInterface iface;
  iface.stop_playback();
  ASSERT_EQ(1, iface.received_data.size());
  EXPECT_EQ('s', iface.received_data[0]);
}

TEST(InterfaceTest, test_e_stop)
{
  TestInterface iface;
  iface.emergency_stop();
  ASSERT_EQ(1, iface.received_data.size());
  EXPECT_EQ(0, iface.received_data[0]);
}

TEST(InterfaceTest, test_clear_e_stop)
{
  TestInterface iface;
  iface.clear_emergency_stop();
  ASSERT_EQ(1, iface.received_data.size());
  EXPECT_EQ('c', iface.received_data[0]);
}

TEST(InterfaceTest, test_status_change_on_unsolicited_status_received)
{
  TestInterface iface;
  iface.start();
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
  iface.receive_data(data);

  auto status = iface.get_last_status();
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
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    Log::logger.connect(chan_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
