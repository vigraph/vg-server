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


TEST(ParserTest, TestReadEmptyText)
{
  string input;
  istringstream iss(input);
  Compiler::Parser parser(iss);
  JSON::Value v;
  ASSERT_NO_THROW(v = parser.get_json());
  ASSERT_TRUE(!v);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
