//==========================================================================
// Tests for <clear> module
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"

class ClearTest: public GraphTester
{
public:
  ClearTest()
  {
    loader.load("./vg-module-binary-clear.so");
  }
};

const auto sample_rate = 1;

TEST_F(ClearTest, TestDefaultClearsFirstBit)
{
  auto& tst = add("binary/clear")
              .set("input", Number{1});

  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  tst.connect("output", snk, "input");

  run();

  const auto expected = vector<Number>{0};
  EXPECT_EQ(expected, output);
}

TEST_F(ClearTest, TestClearABit)
{
  auto& tst = add("binary/clear")
              .set("input", Number{8})
              .set("bit", Number{3});

  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  tst.connect("output", snk, "input");

  run();

  const auto expected = vector<Number>{0};
  EXPECT_EQ(expected, output);
}

TEST_F(ClearTest, TestClearABitOnNonZero)
{
  auto& tst = add("binary/clear")
              .set("input", Number{9})
              .set("bit", Number{3});

  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  tst.connect("output", snk, "input");

  run();

  const auto expected = vector<Number>{1};
  EXPECT_EQ(expected, output);
}

TEST_F(ClearTest, TestClearABitThatIsAlreadyCleared)
{
  auto& tst = add("binary/clear")
              .set("input", Number{1})
              .set("bit", Number{3});

  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  tst.connect("output", snk, "input");

  run();

  const auto expected = vector<Number>{1};
  EXPECT_EQ(expected, output);
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
