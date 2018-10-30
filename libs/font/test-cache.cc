//==========================================================================
// ViGraph Font library: test-cache.cc
//
// Tests for face cache
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-font.h"
#include <gtest/gtest.h>

namespace {

using namespace ViGraph;
using namespace ViGraph::Font;

// Hoping all our test systems have this!  Otherwise we'd have to search
// for one...
const char *test_font = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";

TEST(CacheTest, TestLibraryInitialisation)
{
  Cache cache;
  ASSERT_FALSE(!cache);
}

TEST(CacheTest, TestBogusFaceLoadGivesInvalidFace)
{
  Cache cache;
  Face face = cache.load("bogus.ttf");
  ASSERT_TRUE(!face);
}

TEST(CacheTest, TestValidFaceLoad)
{
  Cache cache;
  const Face& face1 = cache.load(test_font);
  ASSERT_FALSE(!face1);

  // Load again and check it's the same pointer
  const Face& face2 = cache.load(test_font);
  ASSERT_EQ(face1, face2);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
