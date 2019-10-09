//==========================================================================
// ViGraph vg-to-json compiler: test-parser.cc
//
// Tests for compiler library
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-compiler.h"
#include <gtest/gtest.h>

namespace {

using namespace ViGraph;
using namespace ObTools;
using namespace std;

TEST(ParserTest, TestReadEmptyGraph)
{
  string input;
  istringstream iss(input);
  Compiler::Parser parser(iss);
  JSON::Value v;
  ASSERT_NO_THROW(v = parser.get_json());
  ASSERT_EQ(JSON::Value::OBJECT, v.type);
  ASSERT_TRUE(v.o.empty());
}

TEST(ParserTest, TestReadSingleElement)
{
  string input(R"(
foo
)");
  istringstream iss(input);
  Compiler::Parser parser(iss);
  JSON::Value v;
  ASSERT_NO_THROW(v = parser.get_json());
  ASSERT_EQ(JSON::Value::OBJECT, v.type);
  const JSON::Value& foo = v["foo"];
  ASSERT_EQ(JSON::Value::OBJECT, foo.type);
  ASSERT_TRUE(foo.o.empty());
}

TEST(ParserTest, TestReadTwoElements)
{
  string input(R"(
foo
bar
)");
  istringstream iss(input);
  Compiler::Parser parser(iss);
  JSON::Value v;
  ASSERT_NO_THROW(v = parser.get_json());
  ASSERT_EQ(JSON::Value::OBJECT, v.type);
  const JSON::Value& foo = v["foo"];
  ASSERT_EQ(JSON::Value::OBJECT, foo.type);
  ASSERT_TRUE(foo.o.empty());
  const JSON::Value& bar = v["bar"];
  ASSERT_EQ(JSON::Value::OBJECT, bar.type);
  ASSERT_TRUE(bar.o.empty());
}

TEST(ParserTest, TestIgnoreComments)
{
  string input(R"(
#comment
foo
bar #another comment
)");
  istringstream iss(input);
  Compiler::Parser parser(iss);
  JSON::Value v;
  ASSERT_NO_THROW(v = parser.get_json());
  ASSERT_EQ(JSON::Value::OBJECT, v.type);
  const JSON::Value& foo = v["foo"];
  ASSERT_EQ(JSON::Value::OBJECT, foo.type);
  ASSERT_TRUE(foo.o.empty());
  const JSON::Value& bar = v["bar"];
  ASSERT_EQ(JSON::Value::OBJECT, bar.type);
  ASSERT_TRUE(bar.o.empty());
}

TEST(ParserTest, TestSetInputs)
{
  string input(R"(
foo i1=42 i2=3.14 i3="Hello"
)");
  istringstream iss(input);
  Compiler::Parser parser(iss);
  JSON::Value v;
  ASSERT_NO_THROW(v = parser.get_json());
  ASSERT_EQ(JSON::Value::OBJECT, v.type);
  const JSON::Value& foo = v["foo"];
  ASSERT_EQ(JSON::Value::OBJECT, foo.type);
  const JSON::Value& inputs = foo["inputs"];
  ASSERT_EQ(JSON::Value::OBJECT, foo.type);

  const JSON::Value& i1 = inputs["i1"];
  ASSERT_EQ(JSON::Value::OBJECT, i1.type);
  const JSON::Value& i1v = i1["value"];
  ASSERT_EQ(JSON::Value::INTEGER, i1v.type);
  EXPECT_EQ(42, i1v.n);

  const JSON::Value& i2 = inputs["i2"];
  ASSERT_EQ(JSON::Value::OBJECT, i2.type);
  const JSON::Value& i2v = i2["value"];
  ASSERT_EQ(JSON::Value::NUMBER, i2v.type);
  EXPECT_DOUBLE_EQ(3.14, i2v.f);

  const JSON::Value& i3 = inputs["i3"];
  ASSERT_EQ(JSON::Value::OBJECT, i3.type);
  const JSON::Value& i3v = i3["value"];
  ASSERT_EQ(JSON::Value::STRING, i3v.type);
  EXPECT_EQ("Hello", i3v.s);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
