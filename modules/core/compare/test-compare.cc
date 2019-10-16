//==========================================================================
// ViGraph dataflow module: core/compare/test-compare.cc
//
// Tests for <compare> control
//
// Copyright (c) 2017-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"
ModuleLoader loader;

const auto sample_rate = 1;

TEST(CompareTest, TestDefaultCompareLowerTriggersLower)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& compare = tester.add("compare")
                        .set("input", -0.5);
  tester.capture_from(compare, "lower");

  tester.run();

  const auto output = tester.get_output();

  // Should be 1 samples at 1.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(1.0, output[0]);
}

TEST(CompareTest, TestDefaultCompareLowerTriggersDoesntTriggerEqual)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& compare = tester.add("compare")
                        .set("input", -0.5);
  tester.capture_from(compare, "equal");

  tester.run();

  const auto output = tester.get_output();

  // Should be 1 samples at 0.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(0.0, output[0]);
}

TEST(CompareTest, TestDefaultCompareLowerDoesntTriggerHigher)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& compare = tester.add("compare")
                        .set("input", -0.5);
  tester.capture_from(compare, "higher");

  tester.run();

  const auto output = tester.get_output();

  // Should be 1 samples at 1.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(0.0, output[0]);
}

TEST(CompareTest, TestDefaultCompareEqualTriggersEqual)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& compare = tester.add("compare");
  tester.capture_from(compare, "equal");

  tester.run();

  const auto output = tester.get_output();

  // Should be 1 samples at 1.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(1.0, output[0]);
}

TEST(CompareTest, TestDefaultCompareEqualTriggersDoesntTriggerLower)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& compare = tester.add("compare");
  tester.capture_from(compare, "lower");

  tester.run();

  const auto output = tester.get_output();

  // Should be 1 samples at 0.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(0.0, output[0]);
}

TEST(CompareTest, TestDefaultCompareEqualDoesntTriggerHigher)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& compare = tester.add("compare");
  tester.capture_from(compare, "higher");

  tester.run();

  const auto output = tester.get_output();

  // Should be 1 samples at 1.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(0.0, output[0]);
}

TEST(CompareTest, TestDefaultCompareHigherTriggersHigher)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& compare = tester.add("compare")
                        .set("input", 0.5);
  tester.capture_from(compare, "higher");

  tester.run();

  const auto output = tester.get_output();

  // Should be 1 samples at 1.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(1.0, output[0]);
}

TEST(CompareTest, TestDefaultCompareHigherTriggersDoesntTriggerEqual)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& compare = tester.add("compare")
                        .set("input", 0.5);
  tester.capture_from(compare, "equal");

  tester.run();

  const auto output = tester.get_output();

  // Should be 1 samples at 0.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(0.0, output[0]);
}

TEST(CompareTest, TestDefaultCompareHigherDoesntTriggerLower)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& compare = tester.add("compare")
                        .set("input", 0.5);
  tester.capture_from(compare, "lower");

  tester.run();

  const auto output = tester.get_output();

  // Should be 1 samples at 1.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(0.0, output[0]);
}

TEST(CompareTest, TestSetValueCompareHigherDoesntTriggerHigher)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& compare = tester.add("compare")
                        .set("value", 0.6)
                        .set("input", 0.5);
  tester.capture_from(compare, "higher");

  tester.run();

  const auto output = tester.get_output();

  // Should be 1 samples at 0.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(0.0, output[0]);
}

TEST(CompareTest, TestDefaultCompareLowerWithOnChangeTriggersLowerOnlyOnce)
{
  GraphTester<double> tester{loader, 2};

  auto& compare = tester.add("compare")
                        .set("input", -1.0)
                        .set("on-change", 1.0);
  tester.capture_from(compare, "lower");

  tester.run();

  const auto output = tester.get_output();

  // Should be 2 samples at 1.0, 0.0
  ASSERT_EQ(2, output.size());
  EXPECT_EQ(1.0, output[0]);
  EXPECT_EQ(0.0, output[1]);
}

TEST(CompareTest, TestDefaultCompareEqualWithOnChangeTriggersEqualOnlyOnce)
{
  GraphTester<double> tester{loader, 2};

  auto& compare = tester.add("compare")
                        .set("input", 0.0)
                        .set("on-change", 1.0);
  tester.capture_from(compare, "equal");

  tester.run();

  const auto output = tester.get_output();

  // Should be 2 samples at 1.0, 0.0
  ASSERT_EQ(2, output.size());
  EXPECT_EQ(1.0, output[0]);
  EXPECT_EQ(0.0, output[1]);
}

TEST(CompareTest, TestDefaultCompareHigherWithOnChangeTriggersHigherOnlyOnce)
{
  GraphTester<double> tester{loader, 2};

  auto& compare = tester.add("compare")
                        .set("input", 0.5)
                        .set("on-change", 1.0);
  tester.capture_from(compare, "higher");

  tester.run();

  const auto output = tester.get_output();

  // Should be 2 samples at 1.0, 0.0
  ASSERT_EQ(2, output.size());
  EXPECT_EQ(1.0, output[0]);
  EXPECT_EQ(0.0, output[1]);
}

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    Log::logger.connect(chan_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  loader.load("./vg-module-core-compare.so");
  return RUN_ALL_TESTS();
}
