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

TEST(ParserTest, TestReadEmptyGraphWithElements)
{
  string input;
  istringstream iss(input);
  Compiler::Parser parser(iss);
  JSON::Value v;
  ASSERT_NO_THROW(v = parser.get_elements_json());
  ASSERT_EQ(JSON::Value::OBJECT, v.type);
  const JSON::Value& elements = v["elements"];
  ASSERT_EQ(JSON::Value::OBJECT, elements.type);
  ASSERT_TRUE(elements.o.empty());
}

TEST(ParserTest, TestReadSingleElement)
{
  string input(R"(
foo: FOO
)");
  istringstream iss(input);
  Compiler::Parser parser(iss);
  JSON::Value v;
  ASSERT_NO_THROW(v = parser.get_json());
  ASSERT_EQ(JSON::Value::OBJECT, v.type);
  const JSON::Value& foo = v["foo"];
  ASSERT_EQ(JSON::Value::OBJECT, foo.type);
  const JSON::Value& type = foo["type"];
  EXPECT_EQ("FOO", type.as_str());
}

TEST(ParserTest, TestReadSingleElementWithDashedName)
{
  string input(R"(
foo-bar: FOO-BAR
)");
  istringstream iss(input);
  Compiler::Parser parser(iss);
  JSON::Value v;
  ASSERT_NO_THROW(v = parser.get_json());
  ASSERT_EQ(JSON::Value::OBJECT, v.type);
  const JSON::Value& foo = v["foo-bar"];
  ASSERT_EQ(JSON::Value::OBJECT, foo.type);
  const JSON::Value& type = foo["type"];
  EXPECT_EQ("FOO-BAR", type.as_str());
}

TEST(ParserTest, TestReadTwoElements)
{
  string input(R"(
foo: FOO
bar: BAR
)");
  istringstream iss(input);
  Compiler::Parser parser(iss);
  JSON::Value v;
  ASSERT_NO_THROW(v = parser.get_json());
  ASSERT_EQ(JSON::Value::OBJECT, v.type);
  const JSON::Value& foo = v["foo"];
  ASSERT_EQ(JSON::Value::OBJECT, foo.type);
  const JSON::Value& ftype = foo["type"];
  EXPECT_EQ("FOO", ftype.as_str());

  const JSON::Value& bar = v["bar"];
  ASSERT_EQ(JSON::Value::OBJECT, bar.type);
  const JSON::Value& btype = bar["type"];
  EXPECT_EQ("BAR", btype.as_str());
}

TEST(ParserTest, TestReadSingleElementWithNoID)
{
  string input(R"(
foo
)");
  istringstream iss(input);
  Compiler::Parser parser(iss);
  JSON::Value v;
  ASSERT_NO_THROW(v = parser.get_json());
  ASSERT_EQ(JSON::Value::OBJECT, v.type);
  const JSON::Value& foo = v["foo1"];
  ASSERT_EQ(JSON::Value::OBJECT, foo.type);
  const JSON::Value& type = foo["type"];
  EXPECT_EQ("foo", type.as_str());
}

TEST(ParserTest, TestReadSingleElementWithSection)
{
  string input(R"(
foo: BAR/FOO
)");
  istringstream iss(input);
  Compiler::Parser parser(iss);
  JSON::Value v;
  ASSERT_NO_THROW(v = parser.get_json());
  ASSERT_EQ(JSON::Value::OBJECT, v.type);
  const JSON::Value& foo = v["foo"];
  ASSERT_EQ(JSON::Value::OBJECT, foo.type);
  const JSON::Value& type = foo["type"];
  EXPECT_EQ("BAR/FOO", type.as_str());
}

TEST(ParserTest, TestReadSingleElementWithDefaultSection)
{
  string input(R"(
foo: FOO
)");
  istringstream iss(input);
  Compiler::Parser parser(iss);
  parser.set_default_section("default");
  JSON::Value v;
  ASSERT_NO_THROW(v = parser.get_json());
  ASSERT_EQ(JSON::Value::OBJECT, v.type);
  const JSON::Value& foo = v["foo"];
  ASSERT_EQ(JSON::Value::OBJECT, foo.type);
  const JSON::Value& type = foo["type"];
  EXPECT_EQ("default/FOO", type.as_str());
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
  const JSON::Value& foo = v["foo1"];
  ASSERT_EQ(JSON::Value::OBJECT, foo.type);
  const JSON::Value& ftype = foo["type"];
  EXPECT_EQ("foo", ftype.as_str());

  const JSON::Value& bar = v["bar1"];
  ASSERT_EQ(JSON::Value::OBJECT, bar.type);
  const JSON::Value& btype = bar["type"];
  EXPECT_EQ("bar", btype.as_str());
}

TEST(ParserTest, TestSetSettings)
{
  string input(R"(
foo i1=42 i2=3.14 i3="Hello"
)");
  istringstream iss(input);
  Compiler::Parser parser(iss);
  JSON::Value v;
  ASSERT_NO_THROW(v = parser.get_json());
  ASSERT_EQ(JSON::Value::OBJECT, v.type);
  const JSON::Value& foo = v["foo1"];
  ASSERT_EQ(JSON::Value::OBJECT, foo.type);
  const JSON::Value& settings = foo["settings"];
  ASSERT_EQ(JSON::Value::OBJECT, settings.type);

  const JSON::Value& i1 = settings["i1"];
  ASSERT_EQ(JSON::Value::OBJECT, i1.type);
  const JSON::Value& i1v = i1["value"];
  ASSERT_EQ(JSON::Value::INTEGER, i1v.type);
  EXPECT_EQ(42, i1v.n);

  const JSON::Value& i2 = settings["i2"];
  ASSERT_EQ(JSON::Value::OBJECT, i2.type);
  const JSON::Value& i2v = i2["value"];
  ASSERT_EQ(JSON::Value::NUMBER, i2v.type);
  EXPECT_DOUBLE_EQ(3.14, i2v.f);

  const JSON::Value& i3 = settings["i3"];
  ASSERT_EQ(JSON::Value::OBJECT, i3.type);
  const JSON::Value& i3v = i3["value"];
  ASSERT_EQ(JSON::Value::STRING, i3v.type);
  EXPECT_EQ("Hello", i3v.s);
}

TEST(ParserTest, TestSetOutputsWithDefaultConnection)
{
  string input(R"(
foo ->- bar
)");
  istringstream iss(input);
  Compiler::Parser parser(iss);
  JSON::Value v;
  ASSERT_NO_THROW(v = parser.get_json());
  ASSERT_EQ(JSON::Value::OBJECT, v.type);
  const JSON::Value& foo = v["foo1"];
  ASSERT_EQ(JSON::Value::OBJECT, foo.type);
  const JSON::Value& outputs = foo["outputs"];
  ASSERT_EQ(JSON::Value::OBJECT, outputs.type);

  const JSON::Value& o1 = outputs["output"];
  ASSERT_EQ(JSON::Value::OBJECT, o1.type);
  const JSON::Value& o1c = o1["connections"];
  ASSERT_EQ(JSON::Value::ARRAY, o1c.type);
  ASSERT_EQ(1, o1c.size());

  const JSON::Value& o1c1 = o1c.get(0);
  ASSERT_EQ(JSON::Value::OBJECT, o1c1.type);
  const JSON::Value& o1i1 = o1c1["input"];
  ASSERT_EQ(JSON::Value::STRING, o1i1.type);
  EXPECT_EQ("input", o1i1.s);
  const JSON::Value& o1e1 = o1c1["element"];
  ASSERT_EQ(JSON::Value::STRING, o1e1.type);
  EXPECT_EQ("bar1", o1e1.s);
}

TEST(ParserTest, TestSetMultipleOutputsWithExplicitIDs)
{
  string input(R"(
foo o1>i1 ->i2 o1>e2.- # Note two from o1
e1: bar
e2: splat
)");
  istringstream iss(input);
  Compiler::Parser parser(iss);
  JSON::Value v;
  ASSERT_NO_THROW(v = parser.get_json());
  ASSERT_EQ(JSON::Value::OBJECT, v.type);
  const JSON::Value& foo = v["foo1"];
  ASSERT_EQ(JSON::Value::OBJECT, foo.type);
  const JSON::Value& outputs = foo["outputs"];
  ASSERT_EQ(JSON::Value::OBJECT, outputs.type);

  const JSON::Value& o1 = outputs["o1"];
  ASSERT_EQ(JSON::Value::OBJECT, o1.type);
  const JSON::Value& o1c = o1["connections"];
  ASSERT_EQ(JSON::Value::ARRAY, o1c.type);
  ASSERT_EQ(2, o1c.size());
  const JSON::Value& o1c1 = o1c.get(0);
  ASSERT_EQ(JSON::Value::OBJECT, o1c1.type);
  const JSON::Value& o1i1 = o1c1["input"];
  ASSERT_EQ(JSON::Value::STRING, o1i1.type);
  EXPECT_EQ("i1", o1i1.s);
  const JSON::Value& o1e1 = o1c1["element"];
  ASSERT_EQ(JSON::Value::STRING, o1e1.type);
  EXPECT_EQ("e1", o1e1.s);

  const JSON::Value& o1c2 = o1c.get(1);
  ASSERT_EQ(JSON::Value::OBJECT, o1c2.type);
  const JSON::Value& o1i2 = o1c2["input"];
  ASSERT_EQ(JSON::Value::STRING, o1i2.type);
  EXPECT_EQ("input", o1i2.s);
  const JSON::Value& o1e2 = o1c2["element"];
  ASSERT_EQ(JSON::Value::STRING, o1e2.type);
  EXPECT_EQ("e2", o1e2.s);

  const JSON::Value& o2 = outputs["output"];
  ASSERT_EQ(JSON::Value::OBJECT, o2.type);
  const JSON::Value& o2c = o2["connections"];
  ASSERT_EQ(JSON::Value::ARRAY, o2c.type);
  ASSERT_EQ(1, o2c.size());
  const JSON::Value& o2c1 = o2c.get(0);
  ASSERT_EQ(JSON::Value::OBJECT, o2c1.type);
  const JSON::Value& o2i1 = o2c1["input"];
  ASSERT_EQ(JSON::Value::STRING, o2i1.type);
  EXPECT_EQ("i2", o2i1.s);
  const JSON::Value& o2e1 = o2c1["element"];
  ASSERT_EQ(JSON::Value::STRING, o2e1.type);
  EXPECT_EQ("e1", o2e1.s);
}

TEST(ParserTest, TestClone)
{
  string input(R"(
clone copies=10
[
  foo
]
)");
  istringstream iss(input);
  Compiler::Parser parser(iss);
  JSON::Value v;
  ASSERT_NO_THROW(v = parser.get_json());
  ASSERT_EQ(JSON::Value::OBJECT, v.type);
  const JSON::Value& clone = v["clone1"];
  ASSERT_EQ(JSON::Value::OBJECT, clone.type);

  const JSON::Value& settings = clone["settings"];
  ASSERT_EQ(JSON::Value::OBJECT, settings.type);

  const JSON::Value& c = settings["copies"];
  ASSERT_EQ(JSON::Value::OBJECT, c.type);
  const JSON::Value& cv = c["value"];
  ASSERT_EQ(JSON::Value::INTEGER, cv.type);
  EXPECT_EQ(10, cv.n);

  const JSON::Value& elements = clone["elements"];
  ASSERT_EQ(JSON::Value::OBJECT, elements.type);
  const JSON::Value& foo = elements["foo1"];
  ASSERT_EQ(JSON::Value::OBJECT, foo.type);
}

TEST(ParserTest, TestCloneInputs)
{
  string input(R"(
bar ->i1
clone
[
  foo
]
)");
  istringstream iss(input);
  Compiler::Parser parser(iss);
  JSON::Value v;
  ASSERT_NO_THROW(v = parser.get_json());
  ASSERT_EQ(JSON::Value::OBJECT, v.type);
  const JSON::Value& clone = v["clone1"];
  ASSERT_EQ(JSON::Value::OBJECT, clone.type);

  const JSON::Value& inputs = clone["inputs"];
  ASSERT_EQ(JSON::Value::OBJECT, inputs.type);
  const JSON::Value& foo = inputs["i1"];
  ASSERT_EQ(JSON::Value::OBJECT, foo.type);
}

TEST(ParserTest, TestSanityCheckFailsOnDanglingOutputs)
{
  string input("foo ->-");
  istringstream iss(input);
  Compiler::Parser parser(iss);
  JSON::Value v;
  ASSERT_THROW(v = parser.get_json(), Compiler::Exception);
}

TEST(ParserTest, TestSanityCheckFailsOnNonExistentOutputs)
{
  string input("foo -> nowhere.-");
  istringstream iss(input);
  Compiler::Parser parser(iss);
  JSON::Value v;
  ASSERT_THROW(v = parser.get_json(), Compiler::Exception);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
