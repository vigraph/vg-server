//==========================================================================
// ViGraph dataflow module: core/binary/test-binary.cc
//
// Tests for <binary> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"
#include <cmath>

static const auto nsamples = 10;

class BinaryTest: public GraphTester
{
public:
  BinaryTest()
  {
    loader.load("./vg-module-maths-binary.so");
  }

  void test_binary(const string& f, const string& second_input_name,
                   Number x, Number y, Number expected)
  {
    const auto exp = vector<Number>(nsamples, expected);
    auto actual = vector<Number>{};

    auto& fn = add("maths/"+f);
    setup(fn);
    fn.set("input", x);
    fn.set(second_input_name, y);

    auto& sink = add_sink(actual, nsamples);
    fn.connect("output", sink, "input");

    run();

    ASSERT_EQ(nsamples, actual.size());
    for(auto i=0u; i<nsamples; i++)
      EXPECT_DOUBLE_EQ(expected, actual[i])
        << get_as_json(f).as_str() << " " << i;
  }
};

TEST_F(BinaryTest, TestMod)
{
  test_binary("mod", "modulus", 42, 10, 2.0);
}

TEST_F(BinaryTest, TestModZero)
{
  test_binary("mod", "modulus", 42, 0, 0);
}

TEST_F(BinaryTest, TestPower)
{
  test_binary("power", "exponent", 2, 16, 65536);
}

TEST_F(BinaryTest, TestAdd)
{
  test_binary("add", "offset", 2, 1, 3);
}

TEST_F(BinaryTest, TestSubtract)
{
  test_binary("subtract", "offset", 2, 1, 1);
}

TEST_F(BinaryTest, TestMultiply)
{
  test_binary("multiply", "factor", 6, 7, 42);
}

TEST_F(BinaryTest, TestDivide)
{
  test_binary("divide", "factor", 42, 6, 7);
}

TEST_F(BinaryTest, TestDivideZeroPositive)
{
  test_binary("divide", "factor", 42, 0, DBL_MAX);
}

TEST_F(BinaryTest, TestDivideZeroNegative)
{
  test_binary("divide", "factor", -42, 0, DBL_MIN);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
