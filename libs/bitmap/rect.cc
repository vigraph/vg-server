//==========================================================================
// ViGraph bitmap library: rect.cc
//
// Implementation of bitmap rectangle operations
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-bitmap.h"
#include <sstream>

namespace ViGraph { namespace Bitmap {

// -------------------------------------------------------------------
// Convert to PPM (P3 ASCII form)
string Rectangle::to_ppm()
{
  ostringstream oss;
  int height = get_height();
  oss << "P3 " << width << " " << height << " 255\n"; // last = max colour val
  for(int i=0; i<height; i++)
  {
    for(int j=0; j<width; j++)
    {
      Colour::RGBA c = get(j, i);
      if (j) oss << "  ";
      oss << (int)(c.r * 255) << " " << (int)(c.g * 255)
          << " " << (int)(c.b * 255);
    }

    oss << endl;
  }

  return oss.str();
}

// -------------------------------------------------------------------
// Read from a ppm.  Throws runtime_error if it can't read it
void Rectangle::read_from_ppm(const string& ppm)
{
  istringstream iss(ppm);
  string tag;
  iss >> tag;
  if (tag != "P3") throw runtime_error("Can't handle this format");

  int height=0, depth=0;
  width = 0;
  iss >> width >> height >> depth;

  if (!width || !height) throw runtime_error("Can't read width or height");
  if (depth != 255) throw runtime_error("Can only handle 255 colour depth");

  for(int i=0; i<width*height; i++)
  {
    if (!iss) throw runtime_error("Pixel data truncated");
    int r=-1, g=-1, b=-1;
    iss >> r >> g >> b;
    if (r < 0 || g < 0 || b < 0) throw runtime_error("Pixel truncated");
    pixels.push_back(Colour::RGBA(r/255.0, g/255.0, b/255.0, 1.0)); // opaque
  }
}

}} // namespaces
