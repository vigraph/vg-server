//==========================================================================
// ViGraph dataflow module: controls/interpolate/test-interpolate.cc
//
// Tests for <interpolate> control
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(InterpolateTest, TestInterpolateWithNoPointsHasNoEffect)
{
  ControlTester tester(loader);
  tester.test("<interpolate property='foo'/>");
  ASSERT_FALSE(tester.target->got("foo"));
}

#if 0
TEST(InterpolateTest, TestInterpolateWithOnePointHasEffect)
{
  ControlTester tester(loader);
  tester.test("<interpolate>"
                "<point foo='1'/>"
              "</interpolate>");
  ASSERT_TRUE(tester.target->got("foo"));
  const auto& sp = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_EQ(1, sp.v.d);
}
#endif

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    Log::logger.connect(chan_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  loader.load("../set/vg-module-core-control-set.so");
  loader.load("./vg-module-core-control-interpolate.so");
  return RUN_ALL_TESTS();
}
