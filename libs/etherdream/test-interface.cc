//==========================================================================
// ViGraph Ether Dream protocol library: test-interface.cc
//
// Tests for whole interface
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
  void send(const vector<uint8_t>& data) override
  {
    data_received.insert(data_received.end(), data.begin(), data.end());
  }

  size_t receive(vector<uint8_t>& data) override
  {
    // Feed bytes one at a time
    if (data_to_send.size())
    {
      data.push_back(data_to_send.front());
      data_to_send.erase(data_to_send.begin());
      return 1;
    }
    else return 0;
  }

public:
  vector<uint8_t> data_to_send;
  vector<uint8_t> data_received;

  // Prime data to send with given data
  void prime(const vector<uint8_t>& data)
  {
    data_to_send.insert(data_to_send.end(), data.begin(), data.end());
  }
};

// OK status responses
const vector<uint8_t> ok_response_idle
{
  'a',    // ACK
  0,      // command

  // status:
  0,      // protocol
  0,      // light engine state: ready
  0,      // playback state: idle
  0,      // source: network
  0, 0,   // light engine flags: none
  1, 0,   // playback flags: underflow
  0, 1,   // source flags = 256
  1, 4,   // buffer fullness = 1025
  0, 0, 1, 0,  // point rate = 63336
  0, 0, 0, 1   // point count = 16M
};

const vector<uint8_t> ok_response_e_stop
{
  'a',    // ACK
  0,      // command

  // status:
  0,      // protocol
  3,      // light engine state: e-stop
  0,      // playback state: idle
  0,      // source: network
  5, 0,   // light engine flags: e-stop network+active
  2, 0,   // playback flags: e-stop
  0, 1,   // source flags = 256
  1, 4,   // buffer fullness = 1025
  0, 0, 1, 0,  // point rate = 63336
  0, 0, 0, 1   // point count = 16M
};

const vector<uint8_t> ok_response_playback
{
  'a',    // ACK
  0,      // command

  // status:
  0,      // protocol
  0,      // light engine state: ready
  2,      // playback state: playing
  0,      // source: network
  0, 0,   // light engine flags: none
  0, 0,   // playback flags: none
  0, 1,   // source flags = 256
  1, 4,   // buffer fullness = 1025
  0, 0, 1, 0,  // point rate = 63336
  0, 0, 0, 1   // point count = 16M
};

const vector<uint8_t> ok_response_prepared
{
  'a',    // ACK
  0,      // command

  // status:
  0,      // protocol
  0,      // light engine state: ready
  1,      // playback state: prepared
  0,      // source: network
  0, 0,   // light engine flags: none
  0, 0,   // playback flags: none
  0, 1,   // source flags = 256
  1, 4,   // buffer fullness = 1025
  0, 0, 1, 0,  // point rate = 63336
  0, 0, 0, 1   // point count = 16M
};

const vector<uint8_t> ok_response_playing
{
  'a',    // ACK
  0,      // command

  // status:
  0,      // protocol
  0,      // light engine state: ready
  2,      // playback state: playing
  0,      // source: network
  0, 0,   // light engine flags: none
  0, 0,   // playback flags: none
  0, 1,   // source flags = 256
  1, 4,   // buffer fullness = 1025
  0, 0, 1, 0,  // point rate = 63336
  0, 0, 0, 1   // point count = 16M
};

// NAK status response
const vector<uint8_t> fail_response
{
  'I',    // ACK
  0,      // command

  // status:
  0,      // protocol
  1,      // light engine state: warmup
  0,      // playback state: idle
  0,      // source: network
  5, 0,   // light engine flags: e-stop network+active
  3, 0,   // playback flags: shutter open, underflow
  0, 1,   // source flags = 256
  1, 4,   // buffer fullness = 1025
  0, 0, 1, 0,  // point rate = 63336
  0, 0, 0, 1   // point count = 16M
};

TEST(InterfaceTest, test_startup_with_ok_responses_get_valid_status)
{
  TestChannel channel;
  channel.prime(ok_response_idle);  // Startup 'response'
  channel.prime(ok_response_idle);  // Ping response

  Interface intf(channel);
  ASSERT_TRUE(intf.start());
  ASSERT_EQ(1, channel.data_received.size());
  EXPECT_EQ('?', channel.data_received[0]);

  // Verify the status received OK
  const auto& status = intf.get_last_status();
  EXPECT_EQ(Status::LightEngineState::ready, status.light_engine_state);
  EXPECT_EQ(Status::PlaybackState::idle, status.playback_state);
  EXPECT_EQ(Status::Source::network, status.source);
  EXPECT_EQ(0, status.light_engine_flags);
  EXPECT_EQ(Status::PlaybackFlags::shutter_open, status.playback_flags);
  EXPECT_EQ(256, status.source_flags);
  EXPECT_EQ(1025, status.buffer_fullness);
  EXPECT_EQ(0x10000, status.point_rate);
  EXPECT_EQ(0x1000000, status.point_count);
}

TEST(InterfaceTest, test_startup_fails_if_no_response)
{
  TestChannel channel;
  Interface intf(channel);
  ASSERT_FALSE(intf.start());
  ASSERT_EQ(0, channel.data_received.size());
}

TEST(InterfaceTest, test_startup_fails_if_initial_response_bad)
{
  TestChannel channel;
  channel.prime(fail_response);  // Startup 'response'

  Interface intf(channel);
  ASSERT_FALSE(intf.start());
  ASSERT_EQ(0, channel.data_received.size());
}

TEST(InterfaceTest, test_startup_fails_if_ping_response_bad)
{
  TestChannel channel;
  channel.prime(ok_response_idle);    // Startup 'response'
  channel.prime(fail_response);  // Ping response

  Interface intf(channel);
  ASSERT_FALSE(intf.start());
  ASSERT_EQ(1, channel.data_received.size());
  EXPECT_EQ('?', channel.data_received[0]);
}

TEST(InterfaceTest, test_get_ready_clears_e_stop_and_prepares)
{
  TestChannel channel;
  channel.prime(ok_response_e_stop);  // Startup 'response'
  channel.prime(ok_response_e_stop);  // Ping response
  channel.prime(ok_response_idle);    // Clear e-stop response
  channel.prime(ok_response_prepared); // Prepare response

  Interface intf(channel);
  ASSERT_TRUE(intf.start());
  ASSERT_TRUE(intf.get_ready());

  ASSERT_EQ(3, channel.data_received.size());
  EXPECT_EQ('?', channel.data_received[0]);  // ping from start
  EXPECT_EQ('c', channel.data_received[1]);  // clear e-stop
  EXPECT_EQ('p', channel.data_received[2]);  // prepare for data
}

TEST(InterfaceTest, test_sending_points)
{
  TestChannel channel;
  channel.prime(ok_response_prepared);       // After send data
  channel.prime(ok_response_playing);        // After start playing

  Interface intf(channel);
  vector<Point> points;
  points.push_back(Point(0,0,Colour::white));

  ASSERT_TRUE(intf.send(points));

  ASSERT_EQ(28, channel.data_received.size());
  EXPECT_EQ('d', channel.data_received[0]);
  // Assume rest of point data is OK - see test-commands for detailed test

  // Check start-playing request
  EXPECT_EQ('b', channel.data_received[21]);
}

TEST(InterfaceTest, test_buffer_points_available_after_startup)
{
  TestChannel channel;
  channel.prime(ok_response_idle);  // Startup 'response'
  channel.prime(ok_response_idle);  // Ping response

  // Note responses claim 1025 fullness

  Interface intf(channel);
  ASSERT_TRUE(intf.start());
  ASSERT_EQ(4000-1025, intf.get_buffer_points_available());
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
