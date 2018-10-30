//==========================================================================
// ViGraph ILDA animation library: test-writer.cc
//
// Tests for ILDA format writer
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-ilda.h"
#include <gtest/gtest.h>
#include <sstream>

namespace {

using namespace ViGraph;
using namespace ViGraph::Geometry;
using namespace ViGraph::ILDA;

TEST(ILDAReaderTest, TestWriteEmptyFrameHeader)
{
  string data("ILDA"
              "\x00\x00\x00\x04"
              "Test"
              "ing1"
              "test"
              ".com"
              "\x00\x00\x00\x00"
              "\x00\x01\x2a\x00", 32);

  ostringstream oss;
  Writer writer(oss);
  ILDA::Frame frame(ILDA::Frame::Format::true_3d, "Testing1", "test.com", 42);
  ASSERT_NO_THROW(writer.write(frame));
  EXPECT_EQ(data, oss.str());
}

TEST(ILDAReaderTest, TestWriteFrame2DTrueColourWithBlanking)
{
  string data("ILDA\x00\x00\x00\x05"
              "Testing2test.com"
              "\x00\x02\x00\x00"
              "\x00\x01\x00\x00"
              // Point 1
              "\x80\x01\x7f\xff"
              "\x00\x7f\x3f\xbf"        // Note B G R
              // Point 2
              "\x00\x00\x00\x00"
              "\xc0\x00\x00\x00", 48);   // Blanked, last point

  ostringstream oss;
  Writer writer(oss);
  ILDA::Frame frame(ILDA::Frame::Format::true_2d, "Testing2", "test.com");
  frame.points.push_back(Point(-0.5, 0.5, Colour::RGB(0.75, 0.25, 0.50)));
  frame.points.push_back(Point(0, 0));
  ASSERT_NO_THROW(writer.write(frame));
  EXPECT_EQ(data, oss.str());
}

TEST(ILDAReaderTest, TestWriteAnimation)
{
  string data("ILDA\x00\x00\x00\x05"
              "Testing2test.com"
              "\x00\x02\x00\x00"
              "\x00\x02\x00\x00"
              // Point 1
              "\x80\x01\x7f\xff"
              "\x00\x7f\x3f\xbf"        // Note B G R
              // Point 2
              "\x00\x00\x00\x00"
              "\xc0\x00\x00\x00"        // Blanked, last point

              // - Frame 2
              "ILDA\x00\x00\x00\x05"
              "Testing2test.com"
              "\x00\x02\x00\x01"
              "\x00\x02\x00\x00"
              // Point 1
              "\x80\x01\x7f\xff"
              "\x00\xff\xff\xff"        // Note B G R
              // Point 2
              "\x00\x00\x00\x00"
              "\xc0\x00\x00\x00"        // Blanked, last point

              // - End frame
              "ILDA\x00\x00\x00\x05"
              "\x00\x00\x00\x00\x00\x00\x00\x00"
              "\x00\x00\x00\x00\x00\x00\x00\x00"
              "\x00\x00\x00\x02"
              "\x00\x02\x00\x00", 128);

  ostringstream oss;
  Writer writer(oss);
  ILDA::Animation animation;
  ILDA::Frame frame1(ILDA::Frame::Format::true_2d, "Testing2", "test.com");
  frame1.points.push_back(Point(-0.5, 0.5, Colour::RGB(0.75, 0.25, 0.50)));
  frame1.points.push_back(Point(0, 0));
  animation.frames.push_back(frame1);
  ILDA::Frame frame2(ILDA::Frame::Format::true_2d, "Testing2", "test.com");
  frame2.points.push_back(Point(-0.5, 0.5, Colour::RGB(1, 1, 1)));
  frame2.points.push_back(Point(0, 0));
  animation.frames.push_back(frame2);

  ASSERT_NO_THROW(writer.write(animation));
  EXPECT_EQ(data, oss.str());
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
