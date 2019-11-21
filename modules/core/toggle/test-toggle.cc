//==========================================================================
// ViGraph dataflow module: core/toggle/test-toggle.cc
//
// Tests for <toggle> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"
#include "vg-waveform.h"
#include <cmath>

class ToggleTest: public GraphTester
{
public:
  ToggleTest()
  {
    loader.load("./vg-module-core-toggle.so");
  }
};

TEST_F(ToggleTest, TestEmptyInput)
{
  auto& tgl = add("core/toggle");
  const auto input = vector<double>{0, 0, 0, 0, 0};
  auto& src = add_source(input);
  auto actualo = vector<double>{};
  auto actuale = vector<double>{};
  auto actuald = vector<double>{};
  auto& sko = add_sink(actualo, input.size());
  auto& ske = add_sink(actuale, input.size());
  auto& skd = add_sink(actuald, input.size());
  src.connect("output", tgl, "input");
  tgl.connect("output", sko, "input");
  tgl.connect("enabled", ske, "input");
  tgl.connect("disabled", skd, "input");

  run();

  const auto expected = vector<double>{0, 0, 0, 0, 0};
  EXPECT_EQ(expected, actualo);
  EXPECT_EQ(expected, actuale);
  EXPECT_EQ(expected, actuald);
}

TEST_F(ToggleTest, TestTurnsOn)
{
  auto& tgl = add("core/toggle");
  const auto input = vector<double>{0, 0, 1, 0, 0};
  auto& src = add_source(input);
  auto actualo = vector<double>{};
  auto actuale = vector<double>{};
  auto actuald = vector<double>{};
  auto& sko = add_sink(actualo, input.size());
  auto& ske = add_sink(actuale, input.size());
  auto& skd = add_sink(actuald, input.size());
  src.connect("output", tgl, "input");
  tgl.connect("output", sko, "input");
  tgl.connect("enabled", ske, "input");
  tgl.connect("disabled", skd, "input");

  run();

  const auto expectedo = vector<double>{0, 0, 1, 1, 1};
  EXPECT_EQ(expectedo, actualo);
  const auto expectede = vector<double>{0, 0, 1, 0, 0};
  EXPECT_EQ(expectede, actuale);
  const auto expectedd = vector<double>{0, 0, 0, 0, 0};
  EXPECT_EQ(expectedd, actuald);
}

TEST_F(ToggleTest, TestTurnsOff)
{
  auto& tgl = add("core/toggle");
  const auto input = vector<double>{0, 1, 0, 1, 0};
  auto& src = add_source(input);
  auto actualo = vector<double>{};
  auto actuale = vector<double>{};
  auto actuald = vector<double>{};
  auto& sko = add_sink(actualo, input.size());
  auto& ske = add_sink(actuale, input.size());
  auto& skd = add_sink(actuald, input.size());
  src.connect("output", tgl, "input");
  tgl.connect("output", sko, "input");
  tgl.connect("enabled", ske, "input");
  tgl.connect("disabled", skd, "input");

  run();

  const auto expectedo = vector<double>{0, 1, 1, 0, 0};
  EXPECT_EQ(expectedo, actualo);
  const auto expectede = vector<double>{0, 1, 0, 0, 0};
  EXPECT_EQ(expectede, actuale);
  const auto expectedd = vector<double>{0, 0, 0, 1, 0};
  EXPECT_EQ(expectedd, actuald);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
