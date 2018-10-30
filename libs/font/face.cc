//==========================================================================
// ViGraph Font library: face.cc
//
// Font face implementation (stub over FreeType FT_Face)
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-font.h"
#include "ot-log.h"
#include "ot-text.h"

namespace ViGraph { namespace Font {

using namespace ObTools;

// Render to points
void Face::render(const string& text, coord_t height,
                  vector<Point>& /*points*/, double /*precision*/) const
{
  if (!handle) return;

  // Convert (assumedly) UTF8 string to Unicode
  vector<wchar_t> unicode;
  Text::UTF8::decode(text, unicode);

  // Lock the face while we set the size (other threads may want other sizes)
  MT::Lock lock(mutex);
  // Our height isn't in pixels - we are vector, but we need to set it to
  // something!
  auto error = FT_Set_Pixel_Sizes(handle, 0, height);
  if (error)
  {
    Log::Error log;
    log << "Can't set size on font face\n";
  }

  // Plot each glyph
  for(const auto& cp: unicode)
  {
    // Look up to glyph index - if not there, it returns 0 which renders to
    // a square
    FT_UInt index = FT_Get_Char_Index(handle, cp);

    // Try to load it
    auto error = FT_Load_Glyph(handle, index, 0);  // ?!! flags
    if (error)
    {
      Log::Error log;
      log << "Failed to load glyph " << index << ": " << error << endl;
      continue;
    }

    // Check format
    if (handle->glyph->format != FT_GLYPH_FORMAT_OUTLINE)
    {
      Log::Error log;
      log << "Glyph " << index << " is not in outline format "
          << "- is this a vector font?\n";
      continue;
    }

    FT_Outline& outline = handle->glyph->outline;
    Log::Detail log;
    log << "Character " << cp << " outline has " << outline.n_contours
        << " contours, " << outline.n_points << " points\n";

    // We don't use FT_Outline_Decompose with its callbacks because it's a bit
    // of a pain to interface back to C++, plus it's good to be in control of
    // point creation (for example, avoiding the zero-size line it produces)
    for(int i=0; i<outline.n_contours; i++)
    {
      log << "Contour " << i+1 << endl;

      vector<Point> controls;
      for(int j=i?outline.contours[i-1]+1:0; j<=outline.contours[i]; j++)
      {
        // Safety check in case they lie about contour indexes
        if (j>=outline.n_points) break;

        FT_Vector& fp = outline.points[j];
        Point p(fp.x/64.0, fp.y/64.0);
        char tag = outline.tags[j];
        bool is_control = !(tag & 1);
        bool is_cubic = (tag & 2);

        log << "Point " << j << ": " << p
            << (is_control?" control":"") << (is_cubic?" cubic":"") << endl;

        if (is_control)
        {
          // Store control point for later
          controls.push_back(p);
        }
        else
        {
          // If we have any controls, it's a quadratic / cubic (depending how
          // many)
          switch (controls.size())
          {
            case 0: // simple line
            default: // too many controls to understand
              log << "LINETO " << p << endl;
              break;

            case 1: // quadratic
              log << "QUADRATICTO " << controls[0] << "," << p << endl;
              break;

            case 2: // cubic
              log << "CUBICTO " << controls[0] << "," << controls[1]
                  << "," << p << endl;
              break;
          }

          controls.clear();
        }
      }
    }
  }
}

}} // namespaces
