//==========================================================================
// ViGraph bitmap library: rect.cc
//
// Implementation of bitmap rectangle operations
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-bitmap.h"
#include <sstream>
#include <algorithm>

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

// -------------------------------------------------------------------
// Convert to simple ASCII form, for testing
string Rectangle::to_ascii()
{
  ostringstream oss;
  int height = get_height();
  oss << width << "x" << height << endl;
  for(int i=0; i<height; i++)
  {
    for(int j=0; j<width; j++)
    {
      Colour::RGBA c = get(j, i);
      if (c.is_transparent())
        oss << "_";
      else if (c.is_black())
        oss << ".";
      else if (c == Colour::white)
        oss << "*";
      else if (c == Colour::red)
        oss << "r";
      else if (c == Colour::green)
        oss << "g";
      else if (c == Colour::blue)
        oss << "b";
      else
        oss << "?";
    }

    oss << endl;
  }

  return oss.str();
}

// -------------------------------------------------------------------
// Fill a set of polygons
// Closes polygons demarcated by blanked points, colour from final point
// Loosely based on Darel Rex Finley's PD C code at
// https://alienryderflex.com/polygon_fill/
void Rectangle::fill_polygons(vector<Geometry::Point>& points)
{
  int height = get_height();
  if (!height) return;

  Colour::RGBA c;

  // Split into independent polygons on blanked points
  auto start = points.begin();
  while (start != points.end())
  {
    auto end = start;
    // Find next blanked point, or end
    for(end++; end<points.end() && end->is_lit(); end++)
      c = end->c;

    // Scanline loop
    for(auto iy=0; iy<height; iy++)
    {
      vector<double> node_xs;

      // Flip and scale Y to bitmap height, recentre on origin
      double y = 0.5-((double)iy+0.5)/height;

      // Find crossing nodes for this polygon
      for(auto it=start; it<end; it++)
      {
        auto next = it;
        if (++next == end) next=start;  // Wrap

        // Check if this crosses the scanline - note also weeds out
        // horizontal lines which are ignored but would otherwise DBZ
        if ((it->y < y && next->y >= y) || (next->y < y && it->y >= y))
        {
          // Get crossing point from slope
          double x = it->x+(next->x-it->x)*(y-it->y)/(next->y-it->y);
          node_xs.push_back(x);
        }
      }

      if (node_xs.empty()) continue;

      // Sort the crossing nodes
      sort(node_xs.begin(), node_xs.end());

      // Fill between the nodes in pairs
      for(auto i=0ul; i<node_xs.size()-1; i+=2)
      {
        // Range to pixel width
        auto ix0 = (int)((node_xs[i  ]+0.5)*width+0.5);
        auto ix1 = (int)((node_xs[i+1]+0.5)*width+0.5);
        if (ix0 >= width || ix1 < 0) continue;  // Off screen
        ix0 = max(ix0, 0);  // Clip
        ix1 = min(ix1, width);
        for(auto ix=ix0; ix<ix1; ix++)  // Plot scanline
          set(ix, iy, c);
      }
    }

    start = end;
  }
}

}} // namespaces
