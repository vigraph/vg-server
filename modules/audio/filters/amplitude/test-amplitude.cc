//==========================================================================
// ViGraph dataflow module: audio/sources/amplitude/test-amplitude.cc
//
// Tests for <amplitude> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module-test.h"
#include "vg-geometry.h"
#include <cmath>

ModuleLoader loader;
using namespace ViGraph::Geometry;

TEST(AmplitudeTest, TestNoWaveform)
{
  GraphTester tester{loader};

  auto vco = tester.add("vco");
  auto amp = tester.add("amplitude");

  vco.connect("default", amp, "default");
  amp.connect_test();

  tester.test();

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_EQ(0, v.d);
}

TEST(AmplitudeTest, TestSquareWave)
{
  GraphTester tester{loader};

  auto vco = tester.add("vco").set("wave", "square").set("freq", 0.00001);
  auto amp = tester.add("amplitude");

  vco.connect("default", amp, "default");
  amp.connect_test();

  tester.test(10);

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_EQ(1.0, v.d);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  loader.load("../../sources/vco/vg-module-audio-source-vco.so");
  loader.load("./vg-module-audio-filter-amplitude.so");
  loader.add_default_section("audio");
  return RUN_ALL_TESTS();
}
