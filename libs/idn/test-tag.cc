//==========================================================================
// ViGraph IDN stream library: test-tag.cc
//
// Tests for IDN tag structure
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-idn.h"
#include <gtest/gtest.h>
#include <sstream>

namespace {

using namespace ViGraph;
using namespace ViGraph::IDN;

TEST(IDNTagTest, TestExplicitConstruction)
{
  Tag tag(1,2,3,4);
  EXPECT_EQ(1, tag.category);
  EXPECT_EQ(2, tag.subcategory);
  EXPECT_EQ(3, tag.identifier);
  EXPECT_EQ(4, tag.parameter);
}

TEST(IDNTagTest, TestConstructionFrom16BitWord)
{
  Tag tag(0x1234);
  EXPECT_EQ(1, tag.category);
  EXPECT_EQ(2, tag.subcategory);
  EXPECT_EQ(3, tag.identifier);
  EXPECT_EQ(4, tag.parameter);
}

TEST(IDNTagTest, TestConversionTo16BitWord)
{
  Tag tag(1,2,3,4);
  EXPECT_EQ(0x1234, tag.to_word());
}

TEST(IDNTagTest, TestBasicTags)
{
  EXPECT_EQ(0x4200, Tags::x.to_word());
  EXPECT_EQ(0x4210, Tags::y.to_word());
  EXPECT_EQ(0x4220, Tags::z.to_word());
  EXPECT_EQ(0x527E, Tags::red.to_word());
  EXPECT_EQ(0x5214, Tags::green.to_word());
  EXPECT_EQ(0x51CC, Tags::blue.to_word());
  EXPECT_EQ(0x4010, Tags::prec16.to_word());
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
