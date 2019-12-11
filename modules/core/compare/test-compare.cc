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
  auto& cmp = add("core/compare")
              .set("input", Number{-0.5});
  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  cmp.connect("is-lower", snk, "input");

  run();

  // Should be 1 samples at 1.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(1.0, output[0]);
}

TEST_F(CompareTest, TestDefaultCompareLowerTriggersDoesntTriggerEqual)
{
  auto& cmp = add("core/compare")
              .set("input", Number{-0.5});
  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  cmp.connect("is-equal", snk, "input");

  run();

  // Should be 1 samples at 0.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(0.0, output[0]);
}

TEST_F(CompareTest, TestDefaultCompareLowerDoesntTriggerHigher)
{
  auto& cmp = add("core/compare")
              .set("input", Number{-0.5});
  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  cmp.connect("is-higher", snk, "input");

  run();

  // Should be 1 samples at 1.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(0.0, output[0]);
}

TEST_F(CompareTest, TestDefaultCompareEqualTriggersEqual)
{
  auto& cmp = add("core/compare");
  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  cmp.connect("is-equal", snk, "input");

  run();

  // Should be 1 samples at 1.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(1.0, output[0]);
}

TEST_F(CompareTest, TestDefaultCompareEqualTriggersDoesntTriggerLower)
{
  auto& cmp = add("core/compare");
  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  cmp.connect("is-lower", snk, "input");

  run();

  // Should be 1 samples at 0.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(0.0, output[0]);
}

TEST_F(CompareTest, TestDefaultCompareEqualDoesntTriggerHigher)
{
  auto& cmp = add("core/compare");
  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  cmp.connect("is-higher", snk, "input");

  run();

  // Should be 1 samples at 1.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(0.0, output[0]);
}

TEST_F(CompareTest, TestDefaultCompareHigherTriggersHigher)
{
  auto& cmp = add("core/compare")
              .set("input", Number{0.5});
  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  cmp.connect("is-higher", snk, "input");

  run();

  // Should be 1 samples at 1.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(1.0, output[0]);
}

TEST_F(CompareTest, TestDefaultCompareHigherTriggersDoesntTriggerEqual)
{
  auto& cmp = add("core/compare")
              .set("input", Number{0.5});
  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  cmp.connect("is-equal", snk, "input");

  run();

  // Should be 1 samples at 0.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(0.0, output[0]);
}

TEST_F(CompareTest, TestDefaultCompareHigherDoesntTriggerLower)
{
  auto& cmp = add("core/compare")
              .set("input", Number{0.5});
  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  cmp.connect("is-lower", snk, "input");

  run();

  // Should be 1 samples at 1.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(0.0, output[0]);
}

TEST_F(CompareTest, TestSetValueCompareHigherDoesntTriggerHigher)
{
  auto& cmp = add("core/compare")
              .set("value", Number{0.6})
              .set("input", Number{0.5});
  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  cmp.connect("is-higher", snk, "input");

  run();

  // Should be 1 samples at 0.0
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(0.0, output[0]);
}

TEST_F(CompareTest, TestDefaultCompareLowerWithOnChangeTriggersLowerOnlyOnce)
{
  auto& cmp = add("core/compare")
              .set("input", Number{-1.0});
  auto output = vector<Trigger>{};
  auto& snk = add_sink(output, 2);
  cmp.connect("went-lower", snk, "input");

  run();

  // Should be 2 samples at 1.0, 0.0
  ASSERT_EQ(2, output.size());
  EXPECT_EQ(1, output[0]);
  EXPECT_EQ(0, output[1]);
}

TEST_F(CompareTest, TestDefaultCompareEqualWithOnChangeTriggersEqualOnlyOnce)
{
  auto& cmp = add("core/compare")
              .set("input", Number{0.0});
  auto output = vector<Trigger>{};
  auto& snk = add_sink(output, 2);
  cmp.connect("went-equal", snk, "input");

  run();

  // Should be 2 samples at 1.0, 0.0
  ASSERT_EQ(2, output.size());
  EXPECT_EQ(1, output[0]);
  EXPECT_EQ(0, output[1]);
}

TEST_F(CompareTest, TestDefaultCompareHigherWithOnChangeTriggersHigherOnlyOnce)
{
  auto& cmp = add("core/compare")
              .set("input", Number{0.5});
  auto output = vector<Trigger>{};
  auto& snk = add_sink(output, 2);
  cmp.connect("went-higher", snk, "input");

  run();

  // Should be 2 samples at 1.0, 0.0
  ASSERT_EQ(2, output.size());
  EXPECT_EQ(1, output[0]);
  EXPECT_EQ(0, output[1]);
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
