//==========================================================================
// ViGraph dataflow module: core/unary/test-unary.cc
//
// Tests for <unary> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"
#include <cmath>

static const double pi = acos(-1);
static const auto nsamples = 10;

class UnaryTest: public GraphTester
{
public:
  UnaryTest()
  {
    loader.load("./vg-module-maths-unary-ops.so");
  }

  void test_unary(const string& f, Number input, Number expected)
  {
    const auto exp = vector<Number>(nsamples, expected);
    auto actual = vector<Number>{};

    auto& fn = add("maths/"+f);
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
};

TEST_F(UnaryTest, TestSin)
{
  test_unary("sin", pi/6, 0.5);
}

TEST_F(UnaryTest, TestCos)
{
  test_unary("cos", pi/3, 0.5);
}

TEST_F(UnaryTest, TestTan)
{
  test_unary("tan", pi/4, 1.0);
}

TEST_F(UnaryTest, TestASin)
{
  test_unary("asin", 0.5, pi/6);
}

TEST_F(UnaryTest, TestACos)
{
  test_unary("acos", 0.5, pi/3);
}

TEST_F(UnaryTest, TestATan)
{
  test_unary("atan", 1.0, pi/4);
}


TEST_F(UnaryTest, TestLog10)
{
  test_unary("log10", 100.0, 2.0);
}

TEST_F(UnaryTest, TestLog)
{
  test_unary("log", exp(1), 1.0);
}

TEST_F(UnaryTest, TestExp10)
{
  test_unary("exp10", 2.0, 100.0);
}

TEST_F(UnaryTest, TestExp)
{
  test_unary("exp", 1.0, exp(1));
}

TEST_F(UnaryTest, TestSqrt)
{
  test_unary("sqrt", 16.0, 4.0);
}

TEST_F(UnaryTest, TestSquare)
{
  test_unary("square", 4.0, 16.0);
}

TEST_F(UnaryTest, TestCube)
{
  test_unary("cube", 4.0, 64.0);
}

TEST_F(UnaryTest, TestInverse)
{
  test_unary("inverse", 10.0, 0.1);
}

TEST_F(UnaryTest, TestInverse0)
{
  test_unary("inverse", 0, DBL_MAX);
}

TEST_F(UnaryTest, TestFloor)
{
  test_unary("floor", pi, 3.0);
}

TEST_F(UnaryTest, TestFloorNegative)
{
  test_unary("floor", -pi, -4.0);
}

TEST_F(UnaryTest, TestCeil)
{
  test_unary("ceil", pi, 4.0);
}

TEST_F(UnaryTest, TestCeilNegative)
{
  test_unary("ceil", -pi, -3.0);
}

TEST_F(UnaryTest, TestRound)
{
  test_unary("round", 0.5, 1.0);
}

TEST_F(UnaryTest, TestRoundNegative)
{
  test_unary("round", -0.5, -1.0);
}

TEST_F(UnaryTest, TestAbs)
{
  test_unary("abs", 0.5, 0.5);
}

TEST_F(UnaryTest, TestAbsNegative)
{
  test_unary("abs", -0.5, 0.5);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
