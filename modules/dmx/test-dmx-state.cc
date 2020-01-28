//==========================================================================
// ViGraph dataflow module: dmx/test-dmx-state.cc
//
// Tests for DMXState structure
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include <gtest/gtest.h>
#include "dmx-module.h"

TEST(DMXStateTest, TestMergeOverlappingHTP)
{
  DMXState state1, state2;
  state1.channels.push_back(1);
  state1.channels.push_back(2);
  state2.channels.push_back(3);
  state1 += state2;
  ASSERT_EQ(2, state1.channels.size());
  EXPECT_EQ(3, state1.channels[0]);
  EXPECT_EQ(2, state1.channels[1]);
}

TEST(DMXStateTest, TestGetAsJSON)
{
  DMXState state;
  state.channels.push_back(1);
  state.channels.push_back(2);
  auto json = get_as_json(state);
  ASSERT_EQ(JSON::Value::ARRAY, json.type);
  ASSERT_EQ(2, json.a.size());
  EXPECT_EQ(1, json.a[0].as_int());
  EXPECT_EQ(2, json.a[1].as_int());
}

TEST(DMXStateTest, TestSetFromJSON)
{
  JSON::Value json{JSON::Value::ARRAY};
  json.add(1);
  json.add(2);
  DMXState state;
  set_from_json(state, json);
  ASSERT_EQ(2, state.channels.size());
  EXPECT_EQ(1, state.channels[0]);
  EXPECT_EQ(2, state.channels[1]);
}

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
