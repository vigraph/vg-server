//==========================================================================
// ViGraph dataflow machine: test-element.cc
//
// Tests for element basics
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-dataflow.h"
#include <gtest/gtest.h>
#include "ot-log.h"

namespace {

#include "test-elements.h"

//==========================================================================
// Test Element
// Just for testing control updates

extern Module test_module;
class TestElement: public Element
{
 public:
  TestElement(const XML::Element& cfg): Element(&test_module, cfg) {}
  double x{0.0};
};

Module test_module
{
  "test",
  "Test",
  "Test element",
  "test",
  {
    { "x", { "Test property", Value::Type::number,
             static_cast<double Element::*>(&TestElement::x),
             true } }
  },
  { }
};

TEST(ElementTest, TestElementConstructionFromXML)
{
  XML::Element xml("element", "id", "foo");
  TestElement e(xml);
  ASSERT_EQ("foo", e.id);
}

TEST(ElementTest, TestElementControlUpdateDirectSetting)
{
  XML::Element xml("element", "id", "foo");
  TestElement e(xml);
  EXPECT_EQ(0.0, e.x);

  // Set directly
  Value v(1.0);
  e.set_property("x", v);
  EXPECT_EQ(1.0, e.x);
}

TEST(ElementTest, TestElementGetJSON)
{
  XML::Element xml("element", "id", "foo");
  TestElement e(xml);
  JSON::Value value = e.get_json();
  ASSERT_EQ(JSON::Value::Type::OBJECT, value.type);
  ASSERT_EQ("foo", value["id"].as_str());
}

} // anonymous namespace

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    Log::logger.connect(chan_out);
  }
  Init::Sequence::run();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
