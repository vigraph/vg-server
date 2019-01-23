//==========================================================================
// ViGraph dataflow module: audio/sources/wav/test-wav.cc
//
// Tests for <wav> source
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module-test.h"
#include "vg-geometry.h"

ModuleLoader loader;
using namespace ViGraph::Geometry;

TEST(FileTest, TestNoFile)
{
  const string& xml = R"(
    <graph>
      <wav/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 2);  // 1 to start, 1 to generate (at 1.0)
  Fragment *fragment = gen.get_fragment();
  ASSERT_FALSE(!fragment);

  // Should be 44100 samples for each of the 2 channels at 0
  EXPECT_EQ(88200, fragment->waveform.size());
  for(auto i=0u; i<fragment->waveform.size(); i++)
    EXPECT_EQ(0.0, fragment->waveform[i]);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  loader.load("./vg-module-audio-source-wav.so");
  return RUN_ALL_TESTS();
}
