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
  TestControl():
    Control(&test_control_module)
  {}
};

TEST(ControlTest, TestConstructionWithNoTarget)
{
  TestControl e;
  const auto& targets = e.get_targets();
  ASSERT_EQ(1, targets.size());
  EXPECT_EQ("", targets.begin()->first);
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
