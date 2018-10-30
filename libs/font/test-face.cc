//==========================================================================
// ViGraph Font library: test-face.cc
//
// Tests for font face
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-font.h"
#include "ot-log.h"
#include <gtest/gtest.h>

namespace {

using namespace ViGraph;
using namespace ViGraph::Font;

// Hoping all our test systems have this!  Otherwise we'd have to search
// for one...
const char *test_font = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";

TEST(CacheTest, TestFaceRender)
{
  Cache cache;
  const Face& face = cache.load(test_font);
  ASSERT_FALSE(!face);

  vector<Point> points;
  face.render("Hello, world", 10, points);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_err = new Log::StreamChannel{&cerr};
    Log::logger.connect(chan_err);
  }

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
