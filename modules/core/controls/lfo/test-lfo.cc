//==========================================================================
// ViGraph dataflow module: core/controls/lfo/test-lfo.cc
//
// Tests for <lfo> control
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(LFOTest, TestZeroPeriodPhase)
{
  GraphTester tester{loader};

  auto lfo = tester.add("lfo").set("wave", "sin")
                              .set("phase", 0.25)
                              .set("period", 0);
  lfo.connect_test();

  tester.test();

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_EQ(1, v.d);
}


int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    Log::logger.connect(chan_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  loader.load("./vg-module-core-control-lfo.so");
  return RUN_ALL_TESTS();
}
