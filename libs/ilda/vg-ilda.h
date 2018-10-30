//==========================================================================
// ViGraph vector graphics: vg-ilda.h
//
// Reader for the ILDA animation file format
// http://www.ilda.com/resources/StandardsDocs/ILDA_IDTF14_rev011.pdf
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VG_ILDA_H
#define __VG_ILDA_H

#include <vector>
#include <string>
#include <istream>
#include "vg-geometry.h"
#include "ot-chan.h"

namespace ViGraph { namespace ILDA {

// Make our lives easier without polluting anyone else
using namespace std;
using namespace ViGraph;
using namespace ObTools;
using namespace ViGraph::Geometry;

//==========================================================================
// Animation frame
// Note index colour frames are converted to true colour by the reader
// Frame number and total are ignored since implicit in the Animation vector
struct Frame
{
  enum class Format
  {
    indexed_3d = 0,
    indexed_2d = 1,
    palette = 2,
    true_3d = 4,
    true_2d = 5
  };
  Format format;
  string name;
  string company;
  int projector;
  vector<Point> points;  // Also used for colours when a palette

  // Constructor
  Frame(): format(Format::true_2d), projector(0) {}
  Frame(Format _format, const string& _name="", const string& _company="",
        int _proj=0):
    format(_format), name(_name), company(_company), projector(_proj) {}
};

//==========================================================================
// A complete animation
struct Animation
{
  vector<Frame> frames;
};

//==========================================================================
// ILDA format reader
class Reader
{
  Channel::StreamReader input;

  // Internal
  Point read_point(bool with_z = false);
  Colour::RGB read_true_colour();
  Colour::RGB read_bgr_true_colour_with_status();
  Colour::RGB read_indexed_colour_with_status(const Frame& palette);

 public:
  //-----------------------------------------------------------------------
  // Constructor on an input stream
  Reader(istream& in): input(in) {}

  //-----------------------------------------------------------------------
  // Read a single ILDA frame
  // Throws runtime_error if it fails
  void read(Frame& frame, const Frame& palette);

  //-----------------------------------------------------------------------
  // Read a full animation
  // Throws runtime_error if it fails
  void read(Animation& animation);

  //-----------------------------------------------------------------------
  // Get the default palette
  static void get_default_palette(Frame& palette);
};

//==========================================================================
// ILDA format writer
class Writer
{
  Channel::StreamWriter output;

  // Internal
  void write_point(const Point& p, bool with_z=false);
  void write_bgr_true_colour_with_status(Colour::RGB c, bool last=false);

 public:
  //-----------------------------------------------------------------------
  // Constructor on an output stream
  Writer(ostream& out): output(out) {}

  //-----------------------------------------------------------------------
  // Write a single ILDA frame
  // Throws runtime_error if it fails
  void write(Frame& frame, int index=0, int total=1);

  //-----------------------------------------------------------------------
  // Write a full animation
  // Throws runtime_error if it fails
  void write(Animation& animation);
};

//==========================================================================
}} //namespaces
#endif // !__VG_ILDA_H
