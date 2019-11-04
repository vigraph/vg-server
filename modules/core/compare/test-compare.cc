//==========================================================================
// ViGraph dataflow module: core/compare/test-compare.cc
//
// Tests for <compare> control
//
// Copyright (c) 2017-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"

class CompareTest: public GraphTester
{
public:
  CompareTest()
  {
    loader.load("./vg-module-core-compare.so");
  }
};

const auto sample_rate = 1;

TEST_F(CompareTest, TestDefaultCompareLowerTriggersLower)
{
  auto& cmp = add("compare")
              .set("input", -0.5);
  auto output = vector<double>{};
  auto& snk = add_sink(output, sample_rate);
  cmp.connect("lower", snk, "input");

  run();

  // Should be 1 samples at 1.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(1.0, output[0]);
}

TEST_F(CompareTest, TestDefaultCompareLowerTriggersDoesntTriggerEqual)
{
  auto& cmp = add("compare")
              .set("input", -0.5);
  auto output = vector<double>{};
  auto& snk = add_sink(output, sample_rate);
  cmp.connect("equal", snk, "input");

  run();

  // Should be 1 samples at 0.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(0.0, output[0]);
}

TEST_F(CompareTest, TestDefaultCompareLowerDoesntTriggerHigher)
{
  auto& cmp = add("compare")
              .set("input", -0.5);
  auto output = vector<double>{};
  auto& snk = add_sink(output, sample_rate);
  cmp.connect("higher", snk, "input");

  run();

  // Should be 1 samples at 1.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(0.0, output[0]);
}

TEST_F(CompareTest, TestDefaultCompareEqualTriggersEqual)
{
  auto& cmp = add("compare");
  auto output = vector<double>{};
  auto& snk = add_sink(output, sample_rate);
  cmp.connect("equal", snk, "input");

  run();

  // Should be 1 samples at 1.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(1.0, output[0]);
}

TEST_F(CompareTest, TestDefaultCompareEqualTriggersDoesntTriggerLower)
{
  auto& cmp = add("compare");
  auto output = vector<double>{};
  auto& snk = add_sink(output, sample_rate);
  cmp.connect("lower", snk, "input");

  run();

  // Should be 1 samples at 0.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(0.0, output[0]);
}

TEST_F(CompareTest, TestDefaultCompareEqualDoesntTriggerHigher)
{
  auto& cmp = add("compare");
  auto output = vector<double>{};
  auto& snk = add_sink(output, sample_rate);
  cmp.connect("higher", snk, "input");

  run();

  // Should be 1 samples at 1.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(0.0, output[0]);
}

TEST_F(CompareTest, TestDefaultCompareHigherTriggersHigher)
{
  auto& cmp = add("compare")
              .set("input", 0.5);
  auto output = vector<double>{};
  auto& snk = add_sink(output, sample_rate);
  cmp.connect("higher", snk, "input");

  run();

  // Should be 1 samples at 1.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(1.0, output[0]);
}

TEST_F(CompareTest, TestDefaultCompareHigherTriggersDoesntTriggerEqual)
{
  auto& cmp = add("compare")
              .set("input", 0.5);
  auto output = vector<double>{};
  auto& snk = add_sink(output, sample_rate);
  cmp.connect("equal", snk, "input");

  run();

  // Should be 1 samples at 0.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(0.0, output[0]);
}

TEST_F(CompareTest, TestDefaultCompareHigherDoesntTriggerLower)
{
  auto& cmp = add("compare")
              .set("input", 0.5);
  auto output = vector<double>{};
  auto& snk = add_sink(output, sample_rate);
  cmp.connect("lower", snk, "input");

  run();

  // Should be 1 samples at 1.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(0.0, output[0]);
}

TEST_F(CompareTest, TestSetValueCompareHigherDoesntTriggerHigher)
{
  auto& cmp = add("compare")
              .set("value", 0.6)
              .set("input", 0.5);
  auto output = vector<double>{};
  auto& snk = add_sink(output, sample_rate);
  cmp.connect("higher", snk, "input");

  run();

  // Should be 1 samples at 0.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(0.0, output[0]);
}

TEST_F(CompareTest, TestDefaultCompareLowerWithOnChangeTriggersLowerOnlyOnce)
{
  auto& cmp = add("compare")
              .set("input", -1.0)
              .set("on-change", true);
  auto output = vector<double>{};
  auto& snk = add_sink(output, 2);
  cmp.connect("lower", snk, "input");

  run();

  // Should be 2 samples at 1.0, 0.0
  ASSERT_EQ(2, output.size());
  EXPECT_EQ(1.0, output[0]);
  EXPECT_EQ(0.0, output[1]);
}

TEST_F(CompareTest, TestDefaultCompareEqualWithOnChangeTriggersEqualOnlyOnce)
{
  auto& cmp = add("compare")
              .set("input", 0.0)
              .set("on-change", true);
  auto output = vector<double>{};
  auto& snk = add_sink(output, 2);
  cmp.connect("equal", snk, "input");

  run();

  // Should be 2 samples at 1.0, 0.0
  ASSERT_EQ(2, output.size());
  EXPECT_EQ(1.0, output[0]);
  EXPECT_EQ(0.0, output[1]);
}

TEST_F(CompareTest, TestDefaultCompareHigherWithOnChangeTriggersHigherOnlyOnce)
{
  auto& cmp = add("compare")
              .set("input", 0.5)
              .set("on-change", true);
  auto output = vector<double>{};
  auto& snk = add_sink(output, 2);
  cmp.connect("higher", snk, "input");

  run();

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
  return RUN_ALL_TESTS();
}
