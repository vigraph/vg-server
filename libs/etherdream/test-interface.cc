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
