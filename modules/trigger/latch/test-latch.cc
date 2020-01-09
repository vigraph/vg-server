//==========================================================================
// ViGraph dataflow module: trigger/latch/test-latch.cc
//
// Tests for <latch> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"
#include "vg-waveform.h"
#include <cmath>

class LatchTest: public GraphTester
{
public:
  LatchTest()
  {
    loader.load("./vg-module-trigger-latch.so");
  }
};

TEST_F(LatchTest, TestEmptyInput)
{
  auto& latch = add("trigger/latch");
  const auto input = vector<Trigger>{0, 0, 0, 0, 0};
  auto& src = add_source(input);
  auto actualo = vector<Number>{};
  auto actuale = vector<Trigger>{};
  auto actuald = vector<Trigger>{};
  auto& sko = add_sink(actualo, input.size());
  auto& ske = add_sink(actuale, input.size());
  auto& skd = add_sink(actuald, input.size());
  src.connect("output", latch, "input");
  latch.connect("output", sko, "input");
  latch.connect("enabled", ske, "input");
  latch.connect("disabled", skd, "input");

  run();

  const auto expected = vector<Number>{0, 0, 0, 0, 0};
  EXPECT_EQ(expected, actualo);
  const auto expected_t = vector<Trigger>{0, 0, 0, 0, 0};
  EXPECT_EQ(expected_t, actuale);
  EXPECT_EQ(expected_t, actuald);
}

TEST_F(LatchTest, TestToggleTurnsOn)
{
  auto& latch = add("trigger/latch");
  const auto input = vector<Trigger>{0, 0, 1, 0, 0};
  auto& src = add_source(input);
  auto actualo = vector<Number>{};
  auto actuale = vector<Trigger>{};
  auto actuald = vector<Trigger>{};
  auto& sko = add_sink(actualo, input.size());
  auto& ske = add_sink(actuale, input.size());
  auto& skd = add_sink(actuald, input.size());
  src.connect("output", latch, "toggle");
  latch.connect("output", sko, "input");
  latch.connect("enabled", ske, "input");
  latch.connect("disabled", skd, "input");

  run();

  const auto expectedo = vector<Number>{0, 0, 1, 1, 1};
  EXPECT_EQ(expectedo, actualo);
  const auto expectede = vector<Trigger>{0, 0, 1, 0, 0};
  EXPECT_EQ(expectede, actuale);
  const auto expectedd = vector<Trigger>{0, 0, 0, 0, 0};
  EXPECT_EQ(expectedd, actuald);
}

TEST_F(LatchTest, TestToggleTurnsOff)
{
  auto& latch = add("trigger/latch");
  const auto input = vector<Trigger>{0, 1, 0, 1, 0};
  auto& src = add_source(input);
  auto actualo = vector<Number>{};
  auto actuale = vector<Trigger>{};
  auto actuald = vector<Trigger>{};
  auto& sko = add_sink(actualo, input.size());
  auto& ske = add_sink(actuale, input.size());
  auto& skd = add_sink(actuald, input.size());
  src.connect("output", latch, "toggle");
  latch.connect("output", sko, "input");
  latch.connect("enabled", ske, "input");
  latch.connect("disabled", skd, "input");

  run();

  const auto expectedo = vector<Number>{0, 1, 1, 0, 0};
  EXPECT_EQ(expectedo, actualo);
  const auto expectede = vector<Trigger>{0, 1, 0, 0, 0};
  EXPECT_EQ(expectede, actuale);
  const auto expectedd = vector<Trigger>{0, 0, 0, 1, 0};
  EXPECT_EQ(expectedd, actuald);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
