//==========================================================================
// Tests for <test> module
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"

class TestTest: public GraphTester
{
public:
  TestTest()
  {
    loader.load("./vg-module-binary-test.so");
  }
};

const auto sample_rate = 10;

TEST_F(TestTest, TestDefaultTestsFirstBit)
{
  auto& tst = add("binary/test");

  const auto input = vector<Number>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  auto& src = add_source(input);
  src.connect("output", tst, "input");

  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  tst.connect("is-set", snk, "input");

  run();

  const auto expected = vector<Number>{0, 1, 0, 1, 0, 1, 0, 1, 0, 1};
  EXPECT_EQ(expected, output);
}

TEST_F(TestTest, TestBitTested)
{
  auto& tst = add("binary/test")
              .set("bit", Number{3});

  const auto input = vector<Number>{0, 1, 2, 4, 8, 16, 32, 64, 128, 256};
  auto& src = add_source(input);
  src.connect("output", tst, "input");

  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  tst.connect("is-set", snk, "input");

  run();

  const auto expected = vector<Number>{0, 0, 0, 0, 1, 0, 0, 0, 0, 0};
  EXPECT_EQ(expected, output);
}

TEST_F(TestTest, TestWentSet)
{
  auto& tst = add("binary/test")
              .set("bit", Number{3});

  const auto input = vector<Number>{0, 1, 2, 4, 8, 0, 1, 2, 4, 8};
  auto& src = add_source(input);
  src.connect("output", tst, "input");

  auto output = vector<Trigger>{};
  auto& snk = add_sink(output, sample_rate);
  tst.connect("went-set", snk, "input");

  run();

  const auto expected = vector<Trigger>{0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
  EXPECT_EQ(expected, output);
}

TEST_F(TestTest, TestWentUnset)
{
  auto& tst = add("binary/test")
              .set("bit", Number{3});

  const auto input = vector<Number>{0, 1, 2, 4, 8, 0, 1, 2, 4, 8};
  auto& src = add_source(input);
  src.connect("output", tst, "input");

  auto output = vector<Trigger>{};
  auto& snk = add_sink(output, sample_rate);
  tst.connect("went-unset", snk, "input");

  run();

  const auto expected = vector<Trigger>{0, 0, 0, 0, 0, 1, 0, 0, 0, 0};
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
