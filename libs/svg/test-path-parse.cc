//==========================================================================
// ViGraph SVG library: test-path-parse.cc
//
// Tests for <path> reading
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-svg.h"
#include <gtest/gtest.h>

namespace {

using namespace ObTools;
using namespace ViGraph;
using namespace ViGraph::SVG;

// Check and log read exception
void read(Path& path, const string& s)
{
  try
  {
    path.read(s);
  }
  catch (runtime_error e)
  {
    FAIL() << "Exception in SVG read: " << e.what() << endl;
    throw e;
  }
}

void read(Path& path, const XML::Element& xml)
{
  try
  {
    path.read(xml);
  }
  catch (runtime_error e)
  {
    FAIL() << "Exception in SVG read: " << e.what() << endl;
    throw e;
  }
}

// Read a partial path command, with M0,0 on front to create segment
void read_part(Path& path, const string& s)
{
  string s2 = "M0,0"+s;
  read(path, s2);

  // Check and remove first command
  ASSERT_EQ(1, path.segments.size());
  Path::Segment& seg = path.segments.front();
  ASSERT_GE(2, seg.commands.size());
  Path::Command& cmd = seg.commands[0];
  ASSERT_EQ(Path::Command::Type::moveto, cmd.type);
  ASSERT_EQ(2, cmd.values.size());
  EXPECT_EQ(0, cmd.values[0]);
  EXPECT_EQ(0, cmd.values[1]);
  ASSERT_TRUE(cmd.is_absolute);
  seg.commands.erase(seg.commands.begin());
}

TEST(PathParseTest, TestEmptyPath)
{
  Path path;
  ASSERT_NO_THROW(read(path, ""));
  ASSERT_TRUE(path.segments.empty());
}

TEST(PathParseTest, TestBadCommand)
{
  Path path;
  ASSERT_THROW(path.read("X"), runtime_error);
}

TEST(PathParseTest, TestMinimalPathAbsolute)
{
  Path path;
  ASSERT_NO_THROW(read(path, "M0,0Z"));

  ASSERT_EQ(1, path.segments.size());
  Path::Segment& seg = path.segments.front();
  ASSERT_EQ(2, seg.commands.size());
  Path::Command& cmd1 = seg.commands[0];
  ASSERT_EQ(Path::Command::Type::moveto, cmd1.type);
  ASSERT_EQ(2, cmd1.values.size());
  EXPECT_EQ(0, cmd1.values[0]);
  EXPECT_EQ(0, cmd1.values[1]);
  ASSERT_TRUE(cmd1.is_absolute);

  Path::Command& cmd2 = seg.commands[1];
  ASSERT_EQ(Path::Command::Type::closepath, cmd2.type);
  ASSERT_TRUE(cmd2.values.empty());
  ASSERT_TRUE(cmd2.is_absolute);
}

TEST(PathParseTest, TestXMLPathWithColour)
{
  XML::Element doc;
  doc.add("path", "d", "M0,0Z")
    .set_attr("style", "foo:bar;stroke:#304050;wibble:splat;");

  Path path;
  ASSERT_NO_THROW(read(path, doc));
  ASSERT_EQ(1, path.segments.size());
  Path::Segment& seg = path.segments.front();
  ASSERT_EQ(2, seg.commands.size());
  ASSERT_EQ(Colour::RGB("#304050"), seg.colour);
}

TEST(PathParseTest, TestRelativeMoveToIsAbsoluteOnlyFirstTime)
{
  Path path;
  ASSERT_NO_THROW(read(path, "m0,0 m1,1"));

  ASSERT_EQ(2, path.segments.size());
  Path::Segment& seg1 = path.segments.front();
  ASSERT_EQ(1, seg1.commands.size());
  Path::Command& cmd1 = seg1.commands[0];
  ASSERT_EQ(Path::Command::Type::moveto, cmd1.type);
  ASSERT_EQ(2, cmd1.values.size());
  EXPECT_EQ(0, cmd1.values[0]);
  EXPECT_EQ(0, cmd1.values[1]);
  ASSERT_TRUE(cmd1.is_absolute);

  Path::Segment& seg2 = path.segments.back();
  ASSERT_EQ(1, seg2.commands.size());
  Path::Command& cmd2 = seg2.commands[0];
  ASSERT_EQ(Path::Command::Type::moveto, cmd2.type);
  ASSERT_EQ(2, cmd2.values.size());
  EXPECT_EQ(1, cmd2.values[0]);
  EXPECT_EQ(1, cmd2.values[1]);
  ASSERT_FALSE(cmd2.is_absolute);
}

TEST(PathParseTest, TestMinimalPathRelative)
{
  Path path;
  ASSERT_NO_THROW(read(path, "m 4.2-33 z"));

  ASSERT_EQ(1, path.segments.size());
  Path::Segment& seg = path.segments.front();
  ASSERT_EQ(2, seg.commands.size());
  Path::Command& cmd1 = seg.commands[0];
  ASSERT_EQ(Path::Command::Type::moveto, cmd1.type);
  ASSERT_EQ(2, cmd1.values.size());
  EXPECT_EQ(4.2, cmd1.values[0]);
  EXPECT_EQ(-33, cmd1.values[1]);
  ASSERT_TRUE(cmd1.is_absolute); // Moveto always is

  Path::Command& cmd2 = seg.commands[1];
  ASSERT_EQ(Path::Command::Type::closepath, cmd2.type);
  ASSERT_TRUE(cmd2.values.empty());
  ASSERT_FALSE(cmd2.is_absolute);
}

TEST(PathParseTest, TestMultipleSegments)
{
  Path path;
  ASSERT_NO_THROW(read(path, "M0,0m1,1z"));

  ASSERT_EQ(2, path.segments.size());
  Path::Segment& seg1 = path.segments.front();
  ASSERT_EQ(1, seg1.commands.size());
  Path::Command& cmd1 = seg1.commands[0];
  ASSERT_EQ(Path::Command::Type::moveto, cmd1.type);
  ASSERT_EQ(2, cmd1.values.size());
  EXPECT_EQ(0, cmd1.values[0]);
  EXPECT_EQ(0, cmd1.values[1]);
  ASSERT_TRUE(cmd1.is_absolute);

  Path::Segment& seg2 = path.segments.back();
  ASSERT_EQ(2, seg2.commands.size());
  Path::Command& cmd2 = seg2.commands[0];
  ASSERT_EQ(Path::Command::Type::moveto, cmd2.type);
  ASSERT_EQ(2, cmd2.values.size());
  EXPECT_EQ(1, cmd2.values[0]);
  EXPECT_EQ(1, cmd2.values[1]);
  ASSERT_FALSE(cmd2.is_absolute);

  Path::Command& cmd3 = seg2.commands[1];
  ASSERT_EQ(Path::Command::Type::closepath, cmd3.type);
  ASSERT_TRUE(cmd3.values.empty());
  ASSERT_FALSE(cmd3.is_absolute);
}

TEST(PathParseTest, TestRepeatedCommands)
{
  Path path;
  ASSERT_NO_THROW(read(path, "M0,0 l1,2,3 4"));

  ASSERT_EQ(1, path.segments.size());
  Path::Segment& seg = path.segments.front();
  ASSERT_EQ(3, seg.commands.size());
  Path::Command& cmd1 = seg.commands[0];
  ASSERT_EQ(Path::Command::Type::moveto, cmd1.type);
  ASSERT_EQ(2, cmd1.values.size());
  EXPECT_EQ(0, cmd1.values[0]);
  EXPECT_EQ(0, cmd1.values[1]);
  ASSERT_TRUE(cmd1.is_absolute);

  Path::Command& cmd2 = seg.commands[1];
  ASSERT_EQ(Path::Command::Type::lineto, cmd2.type);
  ASSERT_EQ(2, cmd2.values.size());
  EXPECT_EQ(1, cmd2.values[0]);
  EXPECT_EQ(2, cmd2.values[1]);
  ASSERT_FALSE(cmd2.is_absolute);

  Path::Command& cmd3 = seg.commands[2];
  ASSERT_EQ(Path::Command::Type::lineto, cmd3.type);
  ASSERT_EQ(2, cmd3.values.size());
  EXPECT_EQ(3, cmd3.values[0]);
  EXPECT_EQ(4, cmd3.values[1]);
  ASSERT_FALSE(cmd3.is_absolute);
}

TEST(PathParseTest, TestRepeatedMoveToSwitchesToLineTo)
{
  Path path;
  ASSERT_NO_THROW(read(path, "M0,0 1,2,3 4"));

  ASSERT_EQ(1, path.segments.size());
  Path::Segment& seg = path.segments.front();
  ASSERT_EQ(3, seg.commands.size());
  Path::Command& cmd1 = seg.commands[0];
  ASSERT_EQ(Path::Command::Type::moveto, cmd1.type);
  ASSERT_EQ(2, cmd1.values.size());
  EXPECT_EQ(0, cmd1.values[0]);
  EXPECT_EQ(0, cmd1.values[1]);
  ASSERT_TRUE(cmd1.is_absolute);

  Path::Command& cmd2 = seg.commands[1];
  ASSERT_EQ(Path::Command::Type::lineto, cmd2.type);
  ASSERT_EQ(2, cmd2.values.size());
  EXPECT_EQ(1, cmd2.values[0]);
  EXPECT_EQ(2, cmd2.values[1]);
  ASSERT_TRUE(cmd2.is_absolute);

  Path::Command& cmd3 = seg.commands[2];
  ASSERT_EQ(Path::Command::Type::lineto, cmd3.type);
  ASSERT_EQ(2, cmd3.values.size());
  EXPECT_EQ(3, cmd3.values[0]);
  EXPECT_EQ(4, cmd3.values[1]);
  ASSERT_TRUE(cmd3.is_absolute);
}

TEST(PathParseTest, TestLineToAbsolute)
{
  Path path;
  ASSERT_NO_THROW(read_part(path, "L1,2"));

  Path::Segment& seg = path.segments.front();
  ASSERT_EQ(1, seg.commands.size());
  Path::Command& cmd = seg.commands[0];
  ASSERT_EQ(Path::Command::Type::lineto, cmd.type);
  ASSERT_EQ(2, cmd.values.size());
  EXPECT_EQ(1, cmd.values[0]);
  EXPECT_EQ(2, cmd.values[1]);
  ASSERT_TRUE(cmd.is_absolute);
}

TEST(PathParseTest, TestLineToRelative)
{
  Path path;
  ASSERT_NO_THROW(read_part(path, "l2.2,-.3"));

  Path::Segment& seg = path.segments.front();
  ASSERT_EQ(1, seg.commands.size());
  Path::Command& cmd = seg.commands[0];
  ASSERT_EQ(Path::Command::Type::lineto, cmd.type);
  ASSERT_EQ(2, cmd.values.size());
  EXPECT_EQ(2.2, cmd.values[0]);
  EXPECT_EQ(-0.3, cmd.values[1]);
  ASSERT_FALSE(cmd.is_absolute);
}

TEST(PathParseTest, TestHorizontalLineToAbsolute)
{
  Path path;
  ASSERT_NO_THROW(read_part(path, "H1"));

  Path::Segment& seg = path.segments.front();
  ASSERT_EQ(1, seg.commands.size());
  Path::Command& cmd = seg.commands[0];
  ASSERT_EQ(Path::Command::Type::horizontal_lineto, cmd.type);
  ASSERT_EQ(1, cmd.values.size());
  EXPECT_EQ(1, cmd.values[0]);
  ASSERT_TRUE(cmd.is_absolute);
}

TEST(PathParseTest, TestHorizontalLineToRelative)
{
  Path path;
  ASSERT_NO_THROW(read_part(path, "h -0.44"));

  Path::Segment& seg = path.segments.front();
  ASSERT_EQ(1, seg.commands.size());
  Path::Command& cmd = seg.commands[0];
  ASSERT_EQ(Path::Command::Type::horizontal_lineto, cmd.type);
  ASSERT_EQ(1, cmd.values.size());
  EXPECT_EQ(-0.44, cmd.values[0]);
  ASSERT_FALSE(cmd.is_absolute);
}

TEST(PathParseTest, TestVerticalLineToAbsolute)
{
  Path path;
  ASSERT_NO_THROW(read_part(path, "V1"));

  Path::Segment& seg = path.segments.front();
  ASSERT_EQ(1, seg.commands.size());
  Path::Command& cmd = seg.commands[0];
  ASSERT_EQ(Path::Command::Type::vertical_lineto, cmd.type);
  ASSERT_EQ(1, cmd.values.size());
  EXPECT_EQ(1, cmd.values[0]);
  ASSERT_TRUE(cmd.is_absolute);
}

TEST(PathParseTest, TestVerticalLineToRelative)
{
  Path path;
  ASSERT_NO_THROW(read_part(path, "v -1e2"));

  Path::Segment& seg = path.segments.front();
  ASSERT_EQ(1, seg.commands.size());
  Path::Command& cmd = seg.commands[0];
  ASSERT_EQ(Path::Command::Type::vertical_lineto, cmd.type);
  ASSERT_EQ(1, cmd.values.size());
  EXPECT_EQ(-100, cmd.values[0]);
  ASSERT_FALSE(cmd.is_absolute);
}

TEST(PathParseTest, TestCurveToAbsolute)
{
  Path path;
  ASSERT_NO_THROW(read_part(path, "C0,1 2 3, 4,5"));

  Path::Segment& seg = path.segments.front();
  ASSERT_EQ(1, seg.commands.size());
  Path::Command& cmd = seg.commands[0];
  ASSERT_EQ(Path::Command::Type::curveto, cmd.type);
  ASSERT_EQ(6, cmd.values.size());
  EXPECT_EQ(0, cmd.values[0]);
  EXPECT_EQ(1, cmd.values[1]);
  EXPECT_EQ(2, cmd.values[2]);
  EXPECT_EQ(3, cmd.values[3]);
  EXPECT_EQ(4, cmd.values[4]);
  EXPECT_EQ(5, cmd.values[5]);
  ASSERT_TRUE(cmd.is_absolute);
}

TEST(PathParseTest, TestCurveToRelative)
{
  Path path;
  ASSERT_NO_THROW(read_part(path, "c0.1 .1,-.2.2-1,0"));

  Path::Segment& seg = path.segments.front();
  ASSERT_EQ(1, seg.commands.size());
  Path::Command& cmd = seg.commands[0];
  ASSERT_EQ(Path::Command::Type::curveto, cmd.type);
  ASSERT_EQ(6, cmd.values.size());
  EXPECT_EQ(0.1,  cmd.values[0]);
  EXPECT_EQ(0.1,  cmd.values[1]);
  EXPECT_EQ(-0.2, cmd.values[2]);
  EXPECT_EQ(0.2,  cmd.values[3]);
  EXPECT_EQ(-1,   cmd.values[4]);
  EXPECT_EQ(0,    cmd.values[5]);
  ASSERT_FALSE(cmd.is_absolute);
}

TEST(PathParseTest, TestSmoothCurveToAbsolute)
{
  Path path;
  ASSERT_NO_THROW(read_part(path, "S0,1 2 3"));

  Path::Segment& seg = path.segments.front();
  ASSERT_EQ(1, seg.commands.size());
  Path::Command& cmd = seg.commands[0];
  ASSERT_EQ(Path::Command::Type::smooth_curveto, cmd.type);
  ASSERT_EQ(4, cmd.values.size());
  EXPECT_EQ(0, cmd.values[0]);
  EXPECT_EQ(1, cmd.values[1]);
  EXPECT_EQ(2, cmd.values[2]);
  EXPECT_EQ(3, cmd.values[3]);
  ASSERT_TRUE(cmd.is_absolute);
}

TEST(PathParseTest, TestSmoothCurveToRelative)
{
  Path path;
  ASSERT_NO_THROW(read_part(path, "s0.1 .1,-.2.2"));

  Path::Segment& seg = path.segments.front();
  ASSERT_EQ(1, seg.commands.size());
  Path::Command& cmd = seg.commands[0];
  ASSERT_EQ(Path::Command::Type::smooth_curveto, cmd.type);
  ASSERT_EQ(4, cmd.values.size());
  EXPECT_EQ(0.1,  cmd.values[0]);
  EXPECT_EQ(0.1,  cmd.values[1]);
  EXPECT_EQ(-0.2, cmd.values[2]);
  EXPECT_EQ(0.2,  cmd.values[3]);
  ASSERT_FALSE(cmd.is_absolute);
}

TEST(PathParseTest, TestQuadraticBezierCurveToAbsolute)
{
  Path path;
  ASSERT_NO_THROW(read_part(path, "Q0,1,2,3"));

  Path::Segment& seg = path.segments.front();
  ASSERT_EQ(1, seg.commands.size());
  Path::Command& cmd = seg.commands[0];
  ASSERT_EQ(Path::Command::Type::quadratic_bezier_curveto, cmd.type);
  ASSERT_EQ(4, cmd.values.size());
  EXPECT_EQ(0, cmd.values[0]);
  EXPECT_EQ(1, cmd.values[1]);
  EXPECT_EQ(2, cmd.values[2]);
  EXPECT_EQ(3, cmd.values[3]);
  ASSERT_TRUE(cmd.is_absolute);
}

TEST(PathParseTest, TestQuadraticBezierCurveToRelative)
{
  Path path;
  ASSERT_NO_THROW(read_part(path, "q0.1 .1,-.2.2"));

  Path::Segment& seg = path.segments.front();
  ASSERT_EQ(1, seg.commands.size());
  Path::Command& cmd = seg.commands[0];
  ASSERT_EQ(Path::Command::Type::quadratic_bezier_curveto, cmd.type);
  ASSERT_EQ(4, cmd.values.size());
  EXPECT_EQ(0.1,  cmd.values[0]);
  EXPECT_EQ(0.1,  cmd.values[1]);
  EXPECT_EQ(-0.2, cmd.values[2]);
  EXPECT_EQ(0.2,  cmd.values[3]);
  ASSERT_FALSE(cmd.is_absolute);
}

TEST(PathParseTest, TestSmoothQuadraticBezierCurveToAbsolute)
{
  Path path;
  ASSERT_NO_THROW(read_part(path, "T0,1"));

  Path::Segment& seg = path.segments.front();
  ASSERT_EQ(1, seg.commands.size());
  Path::Command& cmd = seg.commands[0];
  ASSERT_EQ(Path::Command::Type::smooth_quadratic_bezier_curveto, cmd.type);
  ASSERT_EQ(2, cmd.values.size());
  EXPECT_EQ(0, cmd.values[0]);
  EXPECT_EQ(1, cmd.values[1]);
  ASSERT_TRUE(cmd.is_absolute);
}

TEST(PathParseTest, TestSmoothQuadraticBezierCurveToRelative)
{
  Path path;
  ASSERT_NO_THROW(read_part(path, "t0.1 .1"));

  Path::Segment& seg = path.segments.front();
  ASSERT_EQ(1, seg.commands.size());
  Path::Command& cmd = seg.commands[0];
  ASSERT_EQ(Path::Command::Type::smooth_quadratic_bezier_curveto, cmd.type);
  ASSERT_EQ(2, cmd.values.size());
  EXPECT_EQ(0.1, cmd.values[0]);
  EXPECT_EQ(0.1, cmd.values[1]);
  ASSERT_FALSE(cmd.is_absolute);
}

TEST(PathParseTest, TestEllipticalArcAbsolute)
{
  Path path;
  ASSERT_NO_THROW(read_part(path, "A0,1 -30 1 0 5,6"));

  Path::Segment& seg = path.segments.front();
  ASSERT_EQ(1, seg.commands.size());
  Path::Command& cmd = seg.commands[0];
  ASSERT_EQ(Path::Command::Type::elliptical_arc, cmd.type);
  ASSERT_EQ(7, cmd.values.size());
  EXPECT_EQ(0, cmd.values[0]);
  EXPECT_EQ(1, cmd.values[1]);
  EXPECT_EQ(-30, cmd.values[2]);
  EXPECT_EQ(1, cmd.values[3]);
  EXPECT_EQ(0, cmd.values[4]);
  EXPECT_EQ(5, cmd.values[5]);
  EXPECT_EQ(6, cmd.values[6]);
  ASSERT_TRUE(cmd.is_absolute);
}

TEST(PathParseTest, TestEllipticalArcRelative)
{
  Path path;
  ASSERT_NO_THROW(read_part(path, "a0,1 -30 1 0 5,6"));

  Path::Segment& seg = path.segments.front();
  ASSERT_EQ(1, seg.commands.size());
  Path::Command& cmd = seg.commands[0];
  ASSERT_EQ(Path::Command::Type::elliptical_arc, cmd.type);
  ASSERT_EQ(7, cmd.values.size());
  EXPECT_EQ(0, cmd.values[0]);
  EXPECT_EQ(1, cmd.values[1]);
  EXPECT_EQ(-30, cmd.values[2]);
  EXPECT_EQ(1, cmd.values[3]);
  EXPECT_EQ(0, cmd.values[4]);
  EXPECT_EQ(5, cmd.values[5]);
  EXPECT_EQ(6, cmd.values[6]);
  ASSERT_FALSE(cmd.is_absolute);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
