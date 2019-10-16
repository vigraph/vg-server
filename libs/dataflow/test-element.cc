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

extern SimpleModule test_module;
class TestElement: public SimpleElement
{
private:
  TestElement *create_clone() const override
  {
    return new TestElement{};
  }
public:
  TestElement(): SimpleElement(test_module) {}
  Input<double> x;
};

SimpleModule test_module
{
  "test",
  "Test",
  "test",
  {},
  {
    { "x", &TestElement::x }
  },
  { }
};

TEST(ElementTest, TestElementControlUpdateDirectSetting)
{
  TestElement e;
  EXPECT_EQ(0.0, e.x.get());

  // Set directly
  e.set("x", 1.0);
  EXPECT_EQ(1.0, e.x.get());
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
