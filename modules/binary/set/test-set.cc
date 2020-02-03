//==========================================================================
// Tests for <set> module
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"

class SetTest: public GraphTester
{
public:
  SetTest()
  {
    loader.load("./vg-module-binary-set.so");
  }
};

const auto sample_rate = 1;

TEST_F(SetTest, TestDefaultSetsFirstBit)
{
  auto& tst = add("binary/set")
              .set("input", Number{});

  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  tst.connect("output", snk, "input");

  run();

  const auto expected = vector<Number>{1};
  EXPECT_EQ(expected, output);
}

TEST_F(SetTest, SetTestABit)
{
  auto& tst = add("binary/set")
              .set("input", Number{})
              .set("bit", Number{3});

  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  tst.connect("output", snk, "input");

  run();

  const auto expected = vector<Number>{8};
  EXPECT_EQ(expected, output);
}

TEST_F(SetTest, SetTestABitOnNonZero)
{
  auto& tst = add("binary/set")
              .set("input", Number{1})
              .set("bit", Number{3});

  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  tst.connect("output", snk, "input");

  run();

  const auto expected = vector<Number>{9};
  EXPECT_EQ(expected, output);
}

TEST_F(SetTest, SetTestABitThatIsAlreadySet)
{
  auto& tst = add("binary/set")
              .set("input", Number{9})
              .set("bit", Number{3});

  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  tst.connect("output", snk, "input");

  run();

  const auto expected = vector<Number>{9};
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
