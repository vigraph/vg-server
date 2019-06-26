//==========================================================================
// ViGraph dataflow module: core/controls/scale/test-scale.cc
//
// Tests for <scale> control
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(ScaleTest, TestDefaultIsOne)
{
  GraphTester tester{loader};

  auto trg = tester.add("trigger");
  auto scl = tester.add("scale");

  trg.connect("output", scl, "inc");
  scl.connect_test();

  tester.test();

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(1.0, v.d, 1e-5);
}

TEST(ScaleTest, TestIncrementScale)
{
  GraphTester tester{loader};

  auto trg = tester.add("trigger");
  auto scl = tester.add("scale").set("factor", 2);

  trg.connect("output", scl, "inc");
  scl.connect_test();

  tester.test();

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(2.0, v.d, 1e-5);
}

TEST(ScaleTest, TestDecrementScale)
{
  GraphTester tester{loader};

  auto trg = tester.add("trigger");
  auto scl = tester.add("scale").set("factor", 2);

  trg.connect("output", scl, "dec");
  scl.connect_test();

  tester.test();

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(0.5, v.d, 1e-5);
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
  loader.load("./vg-module-core-control-scale.so");
  return RUN_ALL_TESTS();
}
