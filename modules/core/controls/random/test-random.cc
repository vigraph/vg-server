//==========================================================================
// ViGraph dataflow module: controls/random/test-random.cc
//
// Tests for <random> control
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(RandomTest, TestDefaultMinMax)
{
  for(int i=0; i<100; i++)
  {
    GraphTester tester{loader};

    auto rnd = tester.add("random");

    rnd.connect_test("value");

    tester.test();

    ASSERT_TRUE(tester.target->got("value"));
    const auto& v = tester.target->get("value");
    ASSERT_EQ(Value::Type::number, v.type);
    EXPECT_LE(0.0, v.d);
    EXPECT_GE(1.0, v.d);
  }
}

TEST(RandomTest, TestSpecifiedMinMax)
{
  for(int i=0; i<100; i++)
  {
    GraphTester tester{loader};

    auto rnd = tester.add("random").set("min", 0.4).set("max", 0.6);

    rnd.connect_test("value");

    tester.test();

    ASSERT_TRUE(tester.target->got("value"));
    const auto& v = tester.target->get("value");
    ASSERT_EQ(Value::Type::number, v.type);
    EXPECT_LE(0.4, v.d);
    EXPECT_GE(0.6, v.d);
  }
}

TEST(RandomTest, TestRandomWithWaitNotTriggeredHasNoEffect)
{
  GraphTester tester{loader};

  auto rnd = tester.add("random").set("wait", true);

  rnd.connect_test("value");

  tester.test();

  ASSERT_FALSE(tester.target->got("value"));
}

TEST(RandomTest, TestRandomWithAutoWaitNotTriggeredHasNoEffect)
{
  GraphTester tester{loader};

  auto trg = tester.add("trigger").set("wait", true);
  auto rnd = tester.add("random");

  trg.connect("trigger", rnd, "trigger");
  rnd.connect_test("value");

  tester.test(2);

  ASSERT_FALSE(tester.target->got("value"));
}

TEST(RandomTest, TestRandomWithWaitTriggeredHasEffect)
{
  GraphTester tester{loader};

  auto trg = tester.add("trigger");
  auto rnd = tester.add("random");

  trg.connect("trigger", rnd, "trigger");
  rnd.connect_test("value");

  tester.test(2);

  ASSERT_TRUE(tester.target->got("value"));
}

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    Log::logger.connect(chan_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  loader.load("../trigger/vg-module-core-control-trigger.so");
  loader.load("./vg-module-core-control-random.so");
  return RUN_ALL_TESTS();
}
