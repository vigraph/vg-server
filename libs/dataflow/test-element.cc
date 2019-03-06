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
class TestElement: public Element
{
 public:
  using Element::Element;
  double x{0.0};

  virtual void set_property(const string& property, const SetParams& sp)
  {
    if (property == "x")
      update_prop(x, sp);
  }
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

TEST(ElementTest, TestElementControlUpdateIncrementSetting)
{
  XML::Element xml("element", "id", "foo");
  TestElement e(xml);

  Value v(1.0);
  Element::SetParams spx(v, true);
  e.x = 0.5;
  e.set_property("x", spx);
  EXPECT_DOUBLE_EQ(1.5, e.x);
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
