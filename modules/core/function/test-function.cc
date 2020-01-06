//==========================================================================
// ViGraph dataflow module: core/function/test-function.cc
//
// Tests for <function> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"
#include <cmath>
#include "func.h"

static const double pi = acos(-1);
static const auto nsamples = 10;

class FunctionTest: public GraphTester
{
public:
  FunctionTest()
  {
    loader.load("./vg-module-core-function.so");
  }

  void test_unary(Func f, Number input, Number expected)
  {
    const auto exp = vector<Number>(nsamples, expected);
    auto actual = vector<Number>{};

    auto& fn = add("core/function")
      .set("f", f);
    setup(fn);
    fn.set("input", input);

    auto& sink = add_sink(actual, nsamples);
    fn.connect("output", sink, "input");

    run();

    ASSERT_EQ(nsamples, actual.size());
    for(auto i=0u; i<nsamples; i++)
      EXPECT_DOUBLE_EQ(expected, actual[i])
        << get_as_json(f).as_str() << " " << i;
  }

  void test_binary(Func f, Number x, Number y, Number expected)
  {
    const auto exp = vector<Number>(nsamples, expected);
    auto actual = vector<Number>{};

    auto& fn = add("core/function")
      .set("f", f);
    setup(fn);
    fn.set("x", x);
    fn.set("y", y);

    auto& sink = add_sink(actual, nsamples);
    fn.connect("output", sink, "input");

    run();

    ASSERT_EQ(nsamples, actual.size());
    for(auto i=0u; i<nsamples; i++)
      EXPECT_DOUBLE_EQ(expected, actual[i])
        << get_as_json(f).as_str() << " " << i;
  }
};


TEST_F(FunctionTest, TestFNotSetIsZero)
{
  const auto expected = vector<Number>(nsamples, 0);
  auto actual = vector<Number>{};

  auto& fn = add("core/function");
  setup(fn);
  fn.set("input", 42.0);

  auto& sink = add_sink(actual, nsamples);
  fn.connect("output", sink, "input");

  run();

  EXPECT_EQ(expected, actual);
}

TEST_F(FunctionTest, TestSin)
{
  test_unary(Func::sin, pi/6, 0.5);
}

TEST_F(FunctionTest, TestCos)
{
  test_unary(Func::cos, pi/3, 0.5);
}

TEST_F(FunctionTest, TestTan)
{
  test_unary(Func::tan, pi/4, 1.0);
}

TEST_F(FunctionTest, TestASin)
{
  test_unary(Func::asin, 0.5, pi/6);
}

TEST_F(FunctionTest, TestACos)
{
  test_unary(Func::acos, 0.5, pi/3);
}

TEST_F(FunctionTest, TestATan)
{
  test_unary(Func::atan, 1.0, pi/4);
}


TEST_F(FunctionTest, TestLog10)
{
  test_unary(Func::log10, 100.0, 2.0);
}

TEST_F(FunctionTest, TestLog)
{
  test_unary(Func::log, exp(1), 1.0);
}

TEST_F(FunctionTest, TestExp10)
{
  test_unary(Func::exp10, 2.0, 100.0);
}

TEST_F(FunctionTest, TestExp)
{
  test_unary(Func::exp, 1.0, exp(1));
}

TEST_F(FunctionTest, TestSqrt)
{
  test_unary(Func::sqrt, 16.0, 4.0);
}

TEST_F(FunctionTest, TestSquare)
{
  test_unary(Func::square, 4.0, 16.0);
}

TEST_F(FunctionTest, TestCube)
{
  test_unary(Func::cube, 4.0, 64.0);
}

TEST_F(FunctionTest, TestInverse)
{
  test_unary(Func::inverse, 10.0, 0.1);
}

TEST_F(FunctionTest, TestInverse0)
{
  test_unary(Func::inverse, 0, DBL_MAX);
}

TEST_F(FunctionTest, TestFloor)
{
  test_unary(Func::floor, pi, 3.0);
}

TEST_F(FunctionTest, TestFloorNegative)
{
  test_unary(Func::floor, -pi, -4.0);
}

TEST_F(FunctionTest, TestCeil)
{
  test_unary(Func::ceil, pi, 4.0);
}

TEST_F(FunctionTest, TestCeilNegative)
{
  test_unary(Func::ceil, -pi, -3.0);
}

TEST_F(FunctionTest, TestRound)
{
  test_unary(Func::round, 0.5, 1.0);
}

TEST_F(FunctionTest, TestRoundNegative)
{
  test_unary(Func::round, -0.5, -1.0);
}

TEST_F(FunctionTest, TestAbs)
{
  test_unary(Func::abs, 0.5, 0.5);
}

TEST_F(FunctionTest, TestAbsNegative)
{
  test_unary(Func::abs, -0.5, 0.5);
}

TEST_F(FunctionTest, TestMod)
{
  test_binary(Func::mod, 42, 10, 2.0);
}

TEST_F(FunctionTest, TestModZero)
{
  test_binary(Func::mod, 42, 0, 0);
}

TEST_F(FunctionTest, TestPower)
{
  test_binary(Func::power, 2, 16, 65536);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
