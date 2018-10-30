//==========================================================================
// ViGraph ILDA animation library: reader.cc
//
// ILDA format reader
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-ilda.h"
#include "ot-chan.h"

namespace ViGraph { namespace ILDA {

//-----------------------------------------------------------------------
// Read a point, 2D or 3D (2 or 3 words)
Point Reader::read_point(bool with_z)
{
  Point p;
  p.x = static_cast<int16_t>(input.read_nbo_16()) / 65536.0;
  p.y = static_cast<int16_t>(input.read_nbo_16()) / 65536.0;
  if (with_z) p.z = static_cast<int16_t>(input.read_nbo_16()) / 65536.0;
  return p;
}

//-----------------------------------------------------------------------
// Read a true colour (3 bytes)
Colour::RGB Reader::read_true_colour()
{
  return Colour::RGB::from_rgb_hex(input.read_byte(),
                                   input.read_byte(),
                                   input.read_byte());
}

//-----------------------------------------------------------------------
// Read a BGR true colour with status code (4 bytes)
Colour::RGB Reader::read_bgr_true_colour_with_status()
{
  unsigned char status = input.read_byte();

  // Ignore last point bit

  if (status & 0x40) // blanking bit
  {
    input.skip(3);   // Ignore RGB values
    return Colour::black;
  }
  else
  {
    // Yes, BGR!  I only noticed the order was reversed reading
    // a Laserboy rant about how it came about...
    unsigned char b = input.read_byte();
    unsigned char g = input.read_byte();
    unsigned char r = input.read_byte();
    return Colour::RGB::from_rgb_hex(r,g,b);
  }
}

//-----------------------------------------------------------------------
// Read an indexed colour with status code, using the given palette to
// convert to true
Colour::RGB Reader::read_indexed_colour_with_status(const Frame& palette)
{
  unsigned char status = input.read_byte();

  // Ignore last point bit

  if (status & 0x40) // blanking bit
  {
    input.skip(1);   // Ignore index value
    return Colour::black;
  }
  else
  {
    unsigned char index = input.read_byte();
    if (index < palette.points.size())
      return palette.points[index].c;
    else
      return Colour::black;  // Safety
  }
}

//-----------------------------------------------------------------------
// Read a single ILDA frame
// Throws runtime_error if it fails
void Reader::read(Frame& frame, const Frame& palette)
{
  try
  {
    string tag;
    input.read(tag, 4);
    if (tag != "ILDA") throw runtime_error("Not an ILDA stream");

    input.skip(3);  // Reserved 5-7
    frame.format = static_cast<Frame::Format>(input.read_byte());

    // Read name and company, chopping at \0
    input.read(frame.name, 8);              // Name 9-16
    size_t n = frame.name.find('\0');
    if (n!=string::npos) frame.name.resize(n);
    input.read(frame.company, 8);           // Company Name 17-24
    n = frame.company.find('\0');
    if (n!=string::npos) frame.company.resize(n);

    uint16_t num_points = input.read_nbo_16();
    frame.points.resize(num_points);

    // Skip frame number and total, not really interested
    input.skip(4);
    frame.projector = input.read_byte(); // Projector 31
    input.skip(1);

    // Read points (colours)
    for(uint16_t i=0; i<num_points; i++)
    {
      Point& p = frame.points[i];
      switch (frame.format)
      {
        case Frame::Format::indexed_3d:
          p = read_point(true);
          p.c = read_indexed_colour_with_status(palette);
        break;

        case Frame::Format::indexed_2d:
          p = read_point();
          p.c = read_indexed_colour_with_status(palette);
        break;

        case Frame::Format::palette:
          p.c = read_true_colour();
          break;

        case Frame::Format::true_3d:
          p = read_point(true);
          p.c = read_bgr_true_colour_with_status();
        break;

        case Frame::Format::true_2d:
          p = read_point();
          p.c = read_bgr_true_colour_with_status();
        break;

        default: throw runtime_error("Unrecognised ILDA format code");
      }
    }
  }
  catch (Channel::Error ce)
  {
    throw runtime_error("ILDA stream error: "+ce.text);
  }
}

//-----------------------------------------------------------------------
// Read a full animation
// Throws runtime_error if it fails
void Reader::read(Animation& animation)
{
  Frame palette;
  get_default_palette(palette);

  // Read frames until an empty one
  for(;;)
  {
    Frame frame;
    read(frame, palette);
    if (!frame.points.size()) break;

    if (frame.format == Frame::Format::palette)
      palette = frame;  // Set new palette
    else
      animation.frames.push_back(frame);
  }
}

//-----------------------------------------------------------------------
// Get the default palette
void Reader::get_default_palette(Frame& palette)
{
  // Suggested default palette from ILDA doc
#define COLOUR(r,g,b) \
  palette.points.push_back(Point(0,0,Colour::RGB::from_rgb_hex(r,g,b)));

  COLOUR(255, 0, 0)
  COLOUR(255, 16, 0)
  COLOUR(255, 32, 0)
  COLOUR(255, 48, 0)
  COLOUR(255, 64, 0)
  COLOUR(255, 80, 0)
  COLOUR(255, 96, 0)
  COLOUR(255, 112, 0)
  COLOUR(255, 128, 0)
  COLOUR(255, 144, 0)
  COLOUR(255, 160, 0)
  COLOUR(255, 176, 0)
  COLOUR(255, 192, 0)
  COLOUR(255, 208, 0)
  COLOUR(255, 224, 0)
  COLOUR(255, 240, 0)
  COLOUR(255, 255, 0)
  COLOUR(224, 255, 0)
  COLOUR(192, 255, 0)
  COLOUR(160, 255, 0)
  COLOUR(128, 255, 0)
  COLOUR(96, 255, 0)
  COLOUR(64, 255, 0)
  COLOUR(32, 255, 0)
  COLOUR(0, 255, 0)
  COLOUR(0, 255, 36)
  COLOUR(0, 255, 73)
  COLOUR(0, 255, 109)
  COLOUR(0, 255, 146)
  COLOUR(0, 255, 182)
  COLOUR(0, 255, 219)
  COLOUR(0, 255, 255)
  COLOUR(0, 227, 255)
  COLOUR(0, 198, 255)
  COLOUR(0, 170, 255)
  COLOUR(0, 142, 255)
  COLOUR(0, 113, 255)
  COLOUR(0, 85, 255)
  COLOUR(0, 56, 255)
  COLOUR(0, 28, 255)
  COLOUR(0, 0, 255)
  COLOUR(32, 0, 255)
  COLOUR(64, 0, 255)
  COLOUR(96, 0, 255)
  COLOUR(128, 0, 255)
  COLOUR(160, 0, 255)
  COLOUR(192, 0, 255)
  COLOUR(224, 0, 255)
  COLOUR(255, 0, 255)
  COLOUR(255, 32, 255)
  COLOUR(255, 64, 255)
  COLOUR(255, 96, 255)
  COLOUR(255, 128, 255)
  COLOUR(255, 160, 255)
  COLOUR(255, 192, 255)
  COLOUR(255, 224, 255)
  COLOUR(255, 255, 255)
  COLOUR(255, 224, 224)
  COLOUR(255, 192, 192)
  COLOUR(255, 160, 160)
  COLOUR(255, 128, 128)
  COLOUR(255, 96, 96)
  COLOUR(255, 64, 64)
  COLOUR(255, 32, 32)
}

}} // namespaces
