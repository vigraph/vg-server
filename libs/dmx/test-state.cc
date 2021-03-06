//==========================================================================
// ViGraph DMX library: test-state.cc
//
// Tests for DMX::State structure
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include <gtest/gtest.h>
#include "vg-dmx.h"

using namespace ViGraph::DMX;

TEST(DMXStateTest, TestSetIndividual)
{
  State state;
  state.set(17, 42);
  ASSERT_EQ(1, state.regions.size());
  ASSERT_TRUE(state.regions.find(17) != state.regions.end());
  ASSERT_EQ(1, state.regions[17].size());
  EXPECT_EQ(42, state.regions[17][0]);
}

TEST(DMXStateTest, TestSetSparse)
{
  State state;
  state.set(17, 42);
  state.set(23, 99);
  ASSERT_EQ(2, state.regions.size());
  ASSERT_TRUE(state.regions.find(17) != state.regions.end());
  ASSERT_EQ(1, state.regions[17].size());
  EXPECT_EQ(42, state.regions[17][0]);
  ASSERT_TRUE(state.regions.find(23) != state.regions.end());
  ASSERT_EQ(1, state.regions[23].size());
  EXPECT_EQ(99, state.regions[23][0]);
}

TEST(DMXStateTest, TestSetExistingNoHTP)
{
  State state;
  state.set(17, 42);
  state.set(17, 7);
  ASSERT_EQ(1, state.regions.size());
  ASSERT_TRUE(state.regions.find(17) != state.regions.end());
  ASSERT_EQ(1, state.regions[17].size());
  EXPECT_EQ(7, state.regions[17][0]);
}

TEST(DMXStateTest, TestSetExistingWithHTPUnder)
{
  State state;
  state.set(17, 42);
  state.set(17, 7, true);
  ASSERT_EQ(1, state.regions.size());
  ASSERT_TRUE(state.regions.find(17) != state.regions.end());
  ASSERT_EQ(1, state.regions[17].size());
  EXPECT_EQ(42, state.regions[17][0]);
}

TEST(DMXStateTest, TestSetExistingWithHTPOver)
{
  State state;
  state.set(17, 42);
  state.set(17, 99, true);
  ASSERT_EQ(1, state.regions.size());
  ASSERT_TRUE(state.regions.find(17) != state.regions.end());
  ASSERT_EQ(1, state.regions[17].size());
  EXPECT_EQ(99, state.regions[17][0]);
}

TEST(DMXStateTest, TestGetIndividual)
{
  State state;
  state.set(17, 42);
  EXPECT_EQ(42, state.get(17));
  EXPECT_EQ(0, state.get(99));
  EXPECT_EQ(7, state.get(99, 7));
}

TEST(DMXStateTest, TestGetInRange)
{
  State state;
  state.set(16, 1);
  state.set(17, 42);
  state.set(18, 2);
  EXPECT_EQ(42, state.get(17));
  EXPECT_EQ(0, state.get(99));
  EXPECT_EQ(7, state.get(99, 7));
}

TEST(DMXStateTest, TestFlatteningSingleUniverse)
{
  State state;
  state.set(0, 1);
  map<int, UniverseData> universes;
  state.flatten(universes);
  ASSERT_EQ(1, universes.size());
  EXPECT_EQ(1, universes[0].channels[0]);
  for(auto i=1u; i<DMX::channels_per_universe; i++)
    EXPECT_EQ(0, universes[0].channels[i]) << i;
}

TEST(DMXStateTest, TestFlatteningMultipleUniverses)
{
  State state;
  state.set(0, 1);
  state.set(511, 2);
  state.set(512, 3);
  state.set(1024, 4);
  map<int, UniverseData> universes;
  state.flatten(universes);
  ASSERT_EQ(3, universes.size());

  EXPECT_EQ(1, universes[0].channels[0]);
  for(auto i=1u; i<511; i++)
    EXPECT_EQ(0, universes[0].channels[i]) << i;
  EXPECT_EQ(2, universes[0].channels[511]);

  EXPECT_EQ(3, universes[1].channels[0]);
  for(auto i=1u; i<512; i++)
    EXPECT_EQ(0, universes[1].channels[i]) << i;

  EXPECT_EQ(4, universes[2].channels[0]);
  for(auto i=1u; i<512; i++)
    EXPECT_EQ(0, universes[2].channels[i]) << i;
}

TEST(DMXStateTest, TestMergeOverlappingHTP)
{
  State state1, state2;
  state1.set(0, 1);
  state1.set(1, 2);
  state2.set(0, 3);
  state1 += state2;
  ASSERT_EQ(1, state1.regions.size());
  ASSERT_TRUE(state1.regions.find(0) != state1.regions.end());
  ASSERT_EQ(2, state1.regions[0].size());
  EXPECT_EQ(3, state1.regions[0][0]);
  EXPECT_EQ(2, state1.regions[0][1]);
}

TEST(DMXStateTest, TestGetAsJSON)
{
  State state;
  state.set(0, 1);
  state.set(1, 2);
  state.set(17, 42);

  auto json = state.get_as_json();
  ASSERT_EQ(JSON::Value::ARRAY, json.type);
  ASSERT_EQ(2, json.a.size());
  const auto& r0 = json[0];
  EXPECT_EQ(0, r0["start"].as_int());
  const auto& v0 = r0["values"];
  ASSERT_EQ(JSON::Value::ARRAY, v0.type);
  ASSERT_EQ(2, v0.a.size());
  EXPECT_EQ(1, v0[0].as_int());
  EXPECT_EQ(2, v0[1].as_int());

  const auto& r17 = json[1];
  EXPECT_EQ(17, r17["start"].as_int());
  const auto& v17 = r17["values"];
  ASSERT_EQ(JSON::Value::ARRAY, v17.type);
  ASSERT_EQ(1, v17.a.size());
  EXPECT_EQ(42, v17[0].as_int());
}

TEST(DMXStateTest, TestSetFromJSON)
{
  JSON::Value json{JSON::Value::ARRAY};
  auto& r0 = json.add(JSON::Value::OBJECT);
  r0.set("start", 0);
  auto& v0 = r0.put("values", JSON::Value::ARRAY);
  v0.add(1);
  v0.add(2);

  auto& r17 = json.add(JSON::Value::OBJECT);
  r17.set("start", 17);
  auto& v17 = r17.put("values", JSON::Value::ARRAY);
  v17.add(42);

  State state;
  state.set_from_json(json);

  ASSERT_EQ(2, state.regions.size());
  ASSERT_TRUE(state.regions.find(0) != state.regions.end());
  ASSERT_EQ(2, state.regions[0].size());
  EXPECT_EQ(1, state.regions[0][0]);
  EXPECT_EQ(2, state.regions[0][1]);
  ASSERT_TRUE(state.regions.find(17) != state.regions.end());
  ASSERT_EQ(1, state.regions[17].size());
  EXPECT_EQ(42, state.regions[17][0]);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
