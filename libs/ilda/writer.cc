//==========================================================================
// ViGraph ILDA animation library: writer.cc
//
// ILDA format writer
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-ilda.h"
#include "ot-chan.h"

namespace ViGraph { namespace ILDA {

//-----------------------------------------------------------------------
// Write a point, 2D or 3D (2 or 3 words)
void Writer::write_point(const Point& p, bool with_z)
{
  // Note we scale by 65535 to ensure both -0.5 and +0.5 can be represented
  output.write_nbo_16(static_cast<uint16_t>(p.x * 65535));
  output.write_nbo_16(static_cast<uint16_t>(p.y * 65535));
  if (with_z) output.write_nbo_16(static_cast<uint16_t>(p.z * 65535));
}

//-----------------------------------------------------------------------
// Write a BGR true colour with status code (4 bytes)
void Writer::write_bgr_true_colour_with_status(Colour::RGB c, bool last)
{
  unsigned char status = (last?0x80:0) | (c.is_black()?0x40:0);
  output.write_byte(status);

  // BGR!
  output.write_byte(static_cast<unsigned char>(c.b*255));
  output.write_byte(static_cast<unsigned char>(c.g*255));
  output.write_byte(static_cast<unsigned char>(c.r*255));
}

//-----------------------------------------------------------------------
// Write a single ILDA frame
// Throws runtime_error if it fails
void Writer::write(Frame& frame, int index, int total)
{
  try
  {
    output.write("ILDA");
    output.skip(3);  // Reserved 5-7
    output.write_byte(static_cast<uint8_t>(frame.format));

    // Write name and company, max 8 chars each
    output.write_fixed(frame.name, 8);
    output.write_fixed(frame.company, 8);

    output.write_nbo_16(frame.points.size());
    output.write_nbo_16(index);
    output.write_nbo_16(total);
    output.write_byte(frame.projector);
    output.skip(1);

    // Write points (colours)
    unsigned int n = frame.points.size();
    for(unsigned int i=0; i<n; i++)
    {
      const Point& p = frame.points[i];
      switch (frame.format)
      {
        case Frame::Format::indexed_3d:
        case Frame::Format::indexed_2d:
        case Frame::Format::palette:
        case Frame::Format::true_3d:
          throw runtime_error("Format not implemented");
        break;

        case Frame::Format::true_2d:
          write_point(p);
          write_bgr_true_colour_with_status(p.c, i==n-1);
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
// Write a full animation
// Throws runtime_error if it fails
void Writer::write(Animation& animation)
{
  // Write all frames
  int n = animation.frames.size();
  for(int i=0; i<n; i++)
    write(animation.frames[i], i, n);

  // Write a blank one to end
  Frame frame;
  write(frame, n, n);
}

}} // namespaces
