//==========================================================================
// ViGraph dataflow machine: test-control.cc
//
// Tests for control construction
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-dataflow.h"
#include <gtest/gtest.h>
#include "ot-log.h"

namespace {

#include "test-elements.h"

//==========================================================================
// Test Control
Module test_control_module
{
  "test-control", "", "", "test",
  { },
  { { "foo", { "FOO", "bar", Value::Type::number }}}
};

class TestControl: public Control
{
 public:
  TestControl(const XML::Element& config):
    Control(&test_control_module, config)
  {}
};

// Test control with multiple properties
Module test_control_multiprop_module
{
  "test-control-multiprop", "", "", "test",
  { },
  {
    { "x", { "EKS", "X", Value::Type::number }},
    { "y", { "WHY", "Y", Value::Type::number }}
  }
};

class TestControlMultiProp: public Control
{
 public:
  TestControlMultiProp(const XML::Element& config):
    Control(&test_control_multiprop_module, config)
  {}
};


TEST(ControlTest, TestConstructionWithNoTarget)
{
  XML::Element xml("control");
  TestControl e(xml);
  const auto& targets = e.get_targets();
  ASSERT_EQ(1, targets.size());
  EXPECT_EQ("", targets.begin()->first);
}

TEST(ControlTest, TestConstructionWithSingleTarget)
{
  XML::Element xml("control", "target", "foo");
  TestControl e(xml);
  const auto& targets = e.get_targets();
  ASSERT_EQ(1, targets.size());
  EXPECT_EQ("foo", targets.begin()->first);
}

TEST(ControlTest, TestConstructionWithMultipleTargets)
{
  XML::Element xml("control", "target", "foo");
  xml.add("target", "id", "bar1");
  xml.add("target", "id", "bar2");

  TestControl e(xml);
  const auto& targets = e.get_targets();
  ASSERT_EQ(3, targets.size());
  EXPECT_FALSE(targets.find("foo") == targets.end());
  EXPECT_FALSE(targets.find("bar1") == targets.end());
  EXPECT_FALSE(targets.find("bar2") == targets.end());
}

TEST(ControlTest, TestConstructionWithSingleProperty)
{
  XML::Element xml("control", "property", "wibble");
  TestControl e(xml);
  const auto& targets = e.get_targets();
  ASSERT_EQ(1, targets.size());
  EXPECT_EQ("", targets.begin()->first);
  const Control::Target& t = targets.begin()->second;
  ASSERT_EQ(1, t.properties.size());
  EXPECT_EQ("foo", t.properties.begin()->first);
  const Control::Property& p = t.properties.begin()->second;
  EXPECT_EQ("wibble", p.name);
  EXPECT_EQ(Value::Type::number, p.type);
}

TEST(ControlTest, TestConstructionWithMultipleTargetsSingleProperty)
{
  XML::Element xml("control");
  xml.add("target", "id", "bar1").set_attr("property", "wibble");
  xml.add("target", "id", "bar2").set_attr("property", "wobble");
  TestControl e(xml);
  const auto& targets = e.get_targets();
  ASSERT_EQ(2, targets.size());

  const auto& it1 = targets.find("bar1");
  ASSERT_NE(targets.end(), it1);
  const Control::Target& t1 = it1->second;
  ASSERT_EQ(1, t1.properties.size());
  EXPECT_EQ("foo", t1.properties.begin()->first);
  const Control::Property& p1 = t1.properties.begin()->second;
  EXPECT_EQ("wibble", p1.name);
  EXPECT_EQ(Value::Type::number, p1.type);

  const auto& it2 = targets.find("bar2");
  ASSERT_NE(targets.end(), it2);
  const Control::Target& t2 = it2->second;
  ASSERT_EQ(1, t2.properties.size());
  EXPECT_EQ("foo", t2.properties.begin()->first);
  const Control::Property& p2 = t2.properties.begin()->second;
  EXPECT_EQ("wobble", p2.name);
  EXPECT_EQ(Value::Type::number, p2.type);
}

TEST(ControlTest, TestConstructionWithMultipleProperties)
{
  XML::Element xml("control");
  xml.set_attr("property-x", "wibble");
  xml.set_attr("property-y", "wobble");
  TestControlMultiProp e(xml);
  const auto& targets = e.get_targets();
  ASSERT_EQ(1, targets.size());
  EXPECT_EQ("", targets.begin()->first);
  const Control::Target& t = targets.begin()->second;
  ASSERT_EQ(2, t.properties.size());

  const auto& it1 = t.properties.find("x");
  ASSERT_NE(t.properties.end(), it1);
  const Control::Property& p1 = it1->second;
  EXPECT_EQ("wibble", p1.name);
  EXPECT_EQ(Value::Type::number, p1.type);

  const auto& it2 = t.properties.find("y");
  ASSERT_NE(t.properties.end(), it2);
  const Control::Property& p2 = it2->second;
  EXPECT_EQ("wobble", p2.name);
  EXPECT_EQ(Value::Type::number, p2.type);
}

TEST(ControlTest, TestConstructionWithPropertiesSplitOverMultipleTargets)
{
  XML::Element xml("control");
  xml.add("target", "id", "bar1").set_attr("property-x", "wibble");
  xml.add("target", "id", "bar2").set_attr("property-y", "wobble");
  TestControlMultiProp e(xml);
  const auto& targets = e.get_targets();
  ASSERT_EQ(2, targets.size());

  const auto& it1 = targets.find("bar1");
  ASSERT_NE(targets.end(), it1);
  const Control::Target& t1 = it1->second;
  ASSERT_EQ(2, t1.properties.size());

  const auto& it11 = t1.properties.find("x");
  ASSERT_NE(t1.properties.end(), it11);
  const Control::Property& p11 = it11->second;
  EXPECT_EQ("wibble", p11.name);
  EXPECT_EQ(Value::Type::number, p11.type);
  EXPECT_TRUE(p11.is_explicit);

  const auto& it12 = t1.properties.find("y");
  ASSERT_NE(t1.properties.end(), it12);
  const Control::Property& p12 = it12->second;
  EXPECT_FALSE(p12.is_explicit);

  const auto& it2 = targets.find("bar2");
  ASSERT_NE(targets.end(), it2);
  const Control::Target& t2 = it2->second;
  ASSERT_EQ(2, t2.properties.size());

  const auto& it21 = t2.properties.find("x");
  ASSERT_NE(t2.properties.end(), it21);
  const Control::Property& p21 = it21->second;
  EXPECT_FALSE(p21.is_explicit);

  const auto& it22 = t2.properties.find("y");
  ASSERT_NE(t2.properties.end(), it22);
  const Control::Property& p22 = it22->second;
  EXPECT_EQ("wobble", p22.name);
  EXPECT_EQ(Value::Type::number, p22.type);

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
