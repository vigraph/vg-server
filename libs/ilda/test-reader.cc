//==========================================================================
// ViGraph ILDA animation library: test-reader.cc
//
// Tests for ILDA format reader
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

TEST(ILDAReaderTest, TestReadEmptyFileThrows)
{
  string data = "";
  istringstream iss(data);
  Reader reader(iss);
  ILDA::Frame frame, palette;
  ASSERT_THROW(reader.read(frame, palette), runtime_error);
}

TEST(ILDAReaderTest, TestReadBogusFileThrows)
{
  string data = "BOGUS";
  istringstream iss(data);
  Reader reader(iss);
  ILDA::Frame frame, palette;
  ASSERT_THROW(reader.read(frame, palette), runtime_error);
}

TEST(ILDAReaderTest, TestReadEmptyFrameHeader)
{
  string data("ILDA"
              "\x00\x00\x00\x04"
              "Test"
              "ing1"
              "test"
              ".com"
              "\x00\x00\x00\x00"
              "\x00\x01\x2a\x00", 32);

  istringstream iss(data);
  Reader reader(iss);
  ILDA::Frame frame, palette;
  ASSERT_NO_THROW(reader.read(frame, palette));
  EXPECT_EQ("Testing1", frame.name);
  EXPECT_EQ("test.com", frame.company);
  EXPECT_EQ(Frame::Format::true_3d, frame.format);
  EXPECT_EQ(42, frame.projector);
  EXPECT_EQ(0, frame.points.size());
}

TEST(ILDAReaderTest, TestReadFrame2DTrueColour)
{
  string data("ILDA\x00\x00\x00\x05"
              "Testing2test.com"
              "\x00\x01\x00\x00"
              "\x00\x01\x00\x00"
              // Point
              "\x80\00\x7f\xff"
              "\x00\x80\x40\xc0", 40);  // Note B G R

  istringstream iss(data);
  Reader reader(iss);
  ILDA::Frame frame, palette;
  ASSERT_NO_THROW(reader.read(frame, palette));
  EXPECT_EQ(Frame::Format::true_2d, frame.format);
  ASSERT_EQ(1, frame.points.size());
  Point& p = frame.points[0];
  EXPECT_NEAR(-0.5, p.x, 0.0001);
  EXPECT_NEAR(0.5, p.y, 0.0001);
  EXPECT_NEAR(0.75, p.c.r, 0.01);
  EXPECT_NEAR(0.25, p.c.g, 0.01);
  EXPECT_NEAR(0.50, p.c.b, 0.01);
}

TEST(ILDAReaderTest, TestReadFrame3DTrueColourWithBlanked)
{
  string data("ILDA\x00\x00\x00\x04"
              "Testing2test.com"
              "\x00\x01\x00\x00"
              "\x00\x01\x00\x00"
              // Point
              "\x80\00\x7f\xff"
              "\x00\00"
              "\x40\x12\x34\x56", 42);

  istringstream iss(data);
  Reader reader(iss);
  ILDA::Frame frame, palette;
  ASSERT_NO_THROW(reader.read(frame, palette));
  EXPECT_EQ(Frame::Format::true_3d, frame.format);
  ASSERT_EQ(1, frame.points.size());
  Point& p = frame.points[0];
  EXPECT_NEAR(-0.5, p.x, 0.0001);
  EXPECT_NEAR(0.5, p.y, 0.0001);
  EXPECT_EQ(0, p.z);
  EXPECT_EQ(0.0,  p.c.r);
  EXPECT_EQ(0.0,  p.c.g);
  EXPECT_EQ(0.0,  p.c.b);
}

TEST(ILDAReaderTest, TestReadFrame2DIndexedColourWithBlankPalette)
{
  string data("ILDA\x00\x00\x00\x01"
              "Testing2test.com"
              "\x00\x01\x00\x00"
              "\x00\x01\x00\x00"
              // Point
              "\x80\00\x7f\xff"
              "\x00\x2a", 38);

  istringstream iss(data);
  Reader reader(iss);
  ILDA::Frame frame, palette;
  ASSERT_NO_THROW(reader.read(frame, palette));
  EXPECT_EQ(Frame::Format::indexed_2d, frame.format);
  ASSERT_EQ(1, frame.points.size());
  Point& p = frame.points[0];
  EXPECT_NEAR(-0.5, p.x, 0.0001);
  EXPECT_NEAR(0.5, p.y, 0.0001);
  EXPECT_EQ(0.0, p.c.r);  // Palette is empty so all points should be blanked
  EXPECT_EQ(0.0, p.c.g);
  EXPECT_EQ(0.0, p.c.b);
}

TEST(ILDAReaderTest, TestReadFrame2DIndexedColourWithValidPalette)
{
  string data("ILDA\x00\x00\x00\x01"
              "Testing2test.com"
              "\x00\x02\x00\x00"
              "\x00\x01\x00\x00"
              // Points
              "\x80\00\x7f\xff"
              "\x00\x00"
              "\x7f\xff\x80\x00"
              "\x00\x01", 44);

  istringstream iss(data);
  Reader reader(iss);
  ILDA::Frame frame, palette;
  palette.points.push_back(Point(0,0, Colour::RGB(0.1, 0.2, 0.3)));
  palette.points.push_back(Point(0,0, Colour::RGB(0.5, 0.6, 0.7)));

  ASSERT_NO_THROW(reader.read(frame, palette));
  EXPECT_EQ(Frame::Format::indexed_2d, frame.format);
  ASSERT_EQ(2, frame.points.size());
  Point& p1 = frame.points[0];
  EXPECT_NEAR(-0.5, p1.x, 0.0001);
  EXPECT_NEAR(0.5, p1.y, 0.0001);
  EXPECT_DOUBLE_EQ(0.1, p1.c.r);
  EXPECT_DOUBLE_EQ(0.2, p1.c.g);
  EXPECT_DOUBLE_EQ(0.3, p1.c.b);
  Point& p2 = frame.points[1];
  EXPECT_NEAR(0.5, p2.x, 0.0001);
  EXPECT_NEAR(-0.5, p2.y, 0.0001);
  EXPECT_DOUBLE_EQ(0.5, p2.c.r);
  EXPECT_DOUBLE_EQ(0.6, p2.c.g);
  EXPECT_DOUBLE_EQ(0.7, p2.c.b);
}

TEST(ILDAReaderTest, TestReadFrame3DIndexedColourWithValidPalette)
{
  string data("ILDA\x00\x00\x00\x00"
              "Testing2test.com"
              "\x00\x02\x00\x00"
              "\x00\x01\x00\x00"
              // Points
              "\x80\00\x7f\xff"
              "\x00\x00\x00\x00"
              "\x7f\xff\x80\x00"
              "\x7f\xff\x00\x01", 48);

  istringstream iss(data);
  Reader reader(iss);
  ILDA::Frame frame, palette;
  palette.points.push_back(Point(0,0, Colour::RGB(0.1, 0.2, 0.3)));
  palette.points.push_back(Point(0,0, Colour::RGB(0.5, 0.6, 0.7)));

  ASSERT_NO_THROW(reader.read(frame, palette));
  EXPECT_EQ(Frame::Format::indexed_3d, frame.format);
  ASSERT_EQ(2, frame.points.size());
  Point& p1 = frame.points[0];
  EXPECT_NEAR(-0.5, p1.x, 0.0001);
  EXPECT_NEAR(0.5, p1.y, 0.0001);
  EXPECT_NEAR(0.0, p1.z, 0.0001);
  EXPECT_DOUBLE_EQ(0.1, p1.c.r);
  EXPECT_DOUBLE_EQ(0.2, p1.c.g);
  EXPECT_DOUBLE_EQ(0.3, p1.c.b);
  Point& p2 = frame.points[1];
  EXPECT_NEAR(0.5, p2.x, 0.0001);
  EXPECT_NEAR(-0.5, p2.y, 0.0001);
  EXPECT_NEAR(0.5, p2.z, 0.0001);
  EXPECT_DOUBLE_EQ(0.5, p2.c.r);
  EXPECT_DOUBLE_EQ(0.6, p2.c.g);
  EXPECT_DOUBLE_EQ(0.7, p2.c.b);
}

TEST(ILDAReaderTest, TestReadPalette)
{
  string data("ILDA\x00\x00\x00\x02"
              "Testing2test.com"
              "\x00\x02\x00\x00"
              "\x00\x01\x00\x00"
              // Colours
              "\x40\x80\xc0"
              "\xff\xff\xff", 38);

  istringstream iss(data);
  Reader reader(iss);
  ILDA::Frame frame, palette;
  ASSERT_NO_THROW(reader.read(frame, palette));
  EXPECT_EQ(Frame::Format::palette, frame.format);
  ASSERT_EQ(2, frame.points.size());
  Colour::RGB& c1 = frame.points[0].c;
  EXPECT_NEAR(0.25, c1.r, 0.01);
  EXPECT_NEAR(0.5,  c1.g, 0.01);
  EXPECT_NEAR(0.75, c1.b, 0.01);
  Colour::RGB& c2 = frame.points[1].c;
  EXPECT_EQ(1.0, c2.r);
  EXPECT_EQ(1.0, c2.g);
  EXPECT_EQ(1.0, c2.b);
}

TEST(ILDAReaderTest, TestDefaultPalette)
{
  ILDA::Frame palette;
  Reader::get_default_palette(palette);
  ASSERT_EQ(64, palette.points.size());

  // Sample critical colours to ensure no missing and RGB order correct
  EXPECT_EQ(Colour::red, palette.points[0].c);
  EXPECT_EQ(Colour::yellow, palette.points[16].c);
  EXPECT_EQ(Colour::green, palette.points[24].c);
  EXPECT_EQ(Colour::cyan, palette.points[31].c);
  EXPECT_EQ(Colour::blue, palette.points[40].c);
  EXPECT_EQ(Colour::magenta, palette.points[48].c);
  EXPECT_EQ(Colour::white, palette.points[56].c);
}

TEST(ILDAReaderTest, TestAnimationWithDefaultPalette)
{
  string data("ILDA\x00\x00\x00\x01"
              "Testing2test.com"
              "\x00\x01\x00\x00"
              "\x00\x01\x00\x00"
              // Point
              "\x80\00\x7f\xff"
              "\x00\x10"

              "ILDA\x00\x00\x00\x01"
              "                "
              "\x00\x00\x00\x00"
              "\x00\x01\x00\x00", 38+32);

  istringstream iss(data);
  Reader reader(iss);
  Animation animation;
  ASSERT_NO_THROW(reader.read(animation));
  ASSERT_EQ(1, animation.frames.size());
  Frame& frame=animation.frames[0];
  EXPECT_EQ(Frame::Format::indexed_2d, frame.format);
  ASSERT_EQ(1, frame.points.size());
  Point& p = frame.points[0];
  EXPECT_NEAR(-0.5, p.x, 0.0001);
  EXPECT_NEAR(0.5, p.y, 0.0001);
  EXPECT_EQ(Colour::yellow, p.c);
}

TEST(ILDAReaderTest, TestAnimationWithExplicitPalette)
{
  string data("ILDA\x00\x00\x00\x02"  // palette
              "Testing2test.com"
              "\x00\x02\x00\x00"
              "\x00\x01\x00\x00"
              // Colours
              "\x40\x80\xc0"
              "\xff\xff\xff"

              "ILDA\x00\x00\x00\x01"  // frame
              "Testing2test.com"
              "\x00\x01\x00\x00"
              "\x00\x01\x00\x00"
              // Point
              "\x80\00\x7f\xff"
              "\x00\x00"  // colour 0

              "ILDA\x00\x00\x00\x01"  // end marker
              "                "
              "\x00\x00\x00\x00"
              "\x00\x01\x00\x00", 38+38+32);

  istringstream iss(data);
  Reader reader(iss);
  Animation animation;
  ASSERT_NO_THROW(reader.read(animation));
  ASSERT_EQ(1, animation.frames.size());
  Frame& frame=animation.frames[0];
  EXPECT_EQ(Frame::Format::indexed_2d, frame.format);
  ASSERT_EQ(1, frame.points.size());
  Point& p = frame.points[0];
  EXPECT_NEAR(-0.5, p.x, 0.0001);
  EXPECT_NEAR(0.5, p.y, 0.0001);
  EXPECT_NEAR(0.25, p.c.r, 0.01);
  EXPECT_NEAR(0.5,  p.c.g, 0.01);
  EXPECT_NEAR(0.75, p.c.b, 0.01);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
