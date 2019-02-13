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

TEST(WavTest, TestNoFile)
{
  const string& xml = R"(
    <graph>
      <wav/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 1);
  Fragment *fragment = gen.get_fragment();
  // Should produce nothing
  EXPECT_TRUE(!fragment);
}

TEST(WavTest, TestFileDefaultFormat)
{
  const auto data = vector<byte>{
    0x52, 0x49, 0x46, 0x46, // "RIFF"
    0x32, 0x00, 0x00, 0x00, // LE chunk size
    0x57, 0x41, 0x56, 0x45, // "WAVE"
    0x66, 0x6d, 0x74, 0x20, // "fmt "
    0x10, 0x00, 0x00, 0x00, // LE Subchunk1 Size
    0x03, 0x00,             // LE Audio Format (IEEE float)
    0x02, 0x00,             // LE Channels (2)
    0x44, 0xac, 0x00, 0x00, // LE Sample Rate (44100)
    0x20, 0x62, 0x05, 0x00, // LE Byte Rate
    0x08, 0x00,             // LE Block Align
    0x20, 0x00,             // LE Bits Per Sample (32)
    0x64, 0x61, 0x74, 0x61, // "data"
    0x18, 0x00, 0x00, 0x00, // LE Subchunk 2 Size
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0, 0
    0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x3f, // 0.5, 0.5
    0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x80, 0x3f  // 1.0, 1.0
  };
  auto f = File::Path{"/tmp/test-float-44100-2.wav"};
  f.write_all(data);
  const string& xml = R"(
    <graph>
      <wav file="/tmp/test-float-44100-2.wav"/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 1);
  Fragment *fragment = gen.get_fragment();
  f.erase();
  ASSERT_FALSE(!fragment);

  // Should be 44100 samples for each of the 2 channels at 0
  EXPECT_EQ(88200, fragment->waveform.size());

  EXPECT_EQ(0.0, fragment->waveform[0]);
  EXPECT_EQ(0.0, fragment->waveform[1]);
  EXPECT_NEAR(0.5, fragment->waveform[2], 0.001);
  EXPECT_NEAR(0.5, fragment->waveform[3], 0.001);
  EXPECT_NEAR(1.0, fragment->waveform[4], 0.001);
  EXPECT_NEAR(1.0, fragment->waveform[5], 0.001);
}

TEST(WavTest, TestFileFormatThatNeedsConversion)
{
  const auto data = vector<byte>{
    0x52, 0x49, 0x46, 0x46, // "RIFF"
    0x2b, 0x00, 0x00, 0x00, // LE chunk size
    0x57, 0x41, 0x56, 0x45, // "WAVE"
    0x66, 0x6d, 0x74, 0x20, // "fmt "
    0x10, 0x00, 0x00, 0x00, // LE Subchunk1 Size
    0x01, 0x00,             // LE Audio Format (PCM)
    0x02, 0x00,             // LE Channels (2)
    0x44, 0xac, 0x00, 0x00, // LE Sample Rate (44100)
    0x10, 0xb1, 0x02, 0x00, // LE Byte Rate
    0x04, 0x00,             // LE Block Align
    0x10, 0x00,             // LE Bits Per Sample (16)
    0x64, 0x61, 0x74, 0x61, // "data"
    0x0c, 0x00, 0x00, 0x00, // LE Subchunk 2 Size
    0x00, 0x00, 0x00, 0x00, // 0, 0
    0x00, 0x40, 0x00, 0x40, // 0.5, 0.5
    0xff, 0x7f, 0xff, 0x7f  // 1.0, 1.0
  };
  auto f = File::Path{"/tmp/test-pcm-44100-2.wav"};
  f.write_all(data);
  const string& xml = R"(
    <graph>
      <wav file="/tmp/test-pcm-44100-2.wav"/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 1);
  Fragment *fragment = gen.get_fragment();
  f.erase();
  ASSERT_FALSE(!fragment);

  // Should be 44100 samples for each of the 2 channels at 0
  EXPECT_EQ(88200, fragment->waveform.size());

  EXPECT_EQ(0.0, fragment->waveform[0]);
  EXPECT_EQ(0.0, fragment->waveform[1]);
  EXPECT_NEAR(0.5, fragment->waveform[2], 0.001);
  EXPECT_NEAR(0.5, fragment->waveform[3], 0.001);
  EXPECT_NEAR(1.0, fragment->waveform[4], 0.001);
  EXPECT_NEAR(1.0, fragment->waveform[5], 0.001);
}

TEST(WavTest, TestLoop)
{
  const auto data = vector<byte>{
    0x52, 0x49, 0x46, 0x46, // "RIFF"
    0x32, 0x00, 0x00, 0x00, // LE chunk size
    0x57, 0x41, 0x56, 0x45, // "WAVE"
    0x66, 0x6d, 0x74, 0x20, // "fmt "
    0x10, 0x00, 0x00, 0x00, // LE Subchunk1 Size
    0x03, 0x00,             // LE Audio Format (IEEE float)
    0x02, 0x00,             // LE Channels (2)
    0x44, 0xac, 0x00, 0x00, // LE Sample Rate (44100)
    0x20, 0x62, 0x05, 0x00, // LE Byte Rate
    0x08, 0x00,             // LE Block Align
    0x20, 0x00,             // LE Bits Per Sample (32)
    0x64, 0x61, 0x74, 0x61, // "data"
    0x18, 0x00, 0x00, 0x00, // LE Subchunk 2 Size
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0, 0
    0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x3f, // 0.5, 0.5
    0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x80, 0x3f  // 1.0, 1.0
  };
  auto f = File::Path{"/tmp/test-float-44100-2.wav"};
  f.write_all(data);
  const string& xml = R"(
    <graph>
      <wav file="/tmp/test-float-44100-2.wav" loop="true"/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 1);
  Fragment *fragment = gen.get_fragment();
  f.erase();
  ASSERT_FALSE(!fragment);

  // Should be 44100 samples for each of the 2 channels at 0
  EXPECT_EQ(88200, fragment->waveform.size());

  for (auto i = 0u; i + 6 < fragment->waveform.size(); i += 6)
  {
    EXPECT_EQ(0.0, fragment->waveform[i]);
    EXPECT_EQ(0.0, fragment->waveform[i + 1]);
    EXPECT_NEAR(0.5, fragment->waveform[i + 2], 0.001);
    EXPECT_NEAR(0.5, fragment->waveform[i + 3], 0.001);
    EXPECT_NEAR(1.0, fragment->waveform[i + 4], 0.001);
    EXPECT_NEAR(1.0, fragment->waveform[i + 5], 0.001);
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  loader.load("./vg-module-audio-source-wav.so");
  return RUN_ALL_TESTS();
}
