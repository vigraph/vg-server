//==========================================================================
// ViGraph colour library: colours.cc
//
// Implementation of colour constructors & conversions
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-colour.h"
#include "ot-text.h"
#include <stdexcept>
#include <map>
#include <sstream>
#include <iomanip>

namespace ViGraph { namespace Colour {

using namespace ObTools;

//-----------------------------------------------------------------------
// Standard colour names
static map<string, string> colour_names = {
  { "aliceblue",                "#F0F8FF" },
  { "antiquewhite",             "#FAEBD7" },
  { "aqua",                     "#00FFFF" },
  { "aquamarine",               "#7FFFD4" },
  { "azure",                    "#F0FFFF" },
  { "beige",                    "#F5F5DC" },
  { "bisque",                   "#FFE4C4" },
  { "black",                    "#000000" },
  { "blanchedalmond",           "#FFEBCD" },
  { "blue",                     "#0000FF" },
  { "blueviolet",               "#8A2BE2" },
  { "brown",                    "#A52A2A" },
  { "burlywood",                "#DEB887" },
  { "cadetblue",                "#5F9EA0" },
  { "chartreuse",               "#7FFF00" },
  { "chocolate",                "#D2691E" },
  { "coral",                    "#FF7F50" },
  { "cornflowerblue",           "#6495ED" },
  { "cornsilk",                 "#FFF8DC" },
  { "crimson",                  "#DC143C" },
  { "cyan",                     "#00FFFF" },
  { "darkblue",                 "#00008B" },
  { "darkcyan",                 "#008B8B" },
  { "darkgoldenrod",            "#B8860B" },
  { "darkgray",                 "#A9A9A9" },
  { "darkgrey",                 "#A9A9A9" },
  { "darkgreen",                "#006400" },
  { "darkkhaki",                "#BDB76B" },
  { "darkmagenta",              "#8B008B" },
  { "darkolivegreen",           "#556B2F" },
  { "darkorange",               "#FF8C00" },
  { "darkorchid",               "#9932CC" },
  { "darkred",                  "#8B0000" },
  { "darksalmon",               "#E9967A" },
  { "darkseagreen",             "#8FBC8F" },
  { "darkslateblue",            "#483D8B" },
  { "darkslategray",            "#2F4F4F" },
  { "darkslategrey",            "#2F4F4F" },
  { "darkturquoise",            "#00CED1" },
  { "darkviolet",               "#9400D3" },
  { "deeppink",                 "#FF1493" },
  { "deepskyblue",              "#00BFFF" },
  { "dimgray",                  "#696969" },
  { "dimgrey",                  "#696969" },
  { "dodgerblue",               "#1E90FF" },
  { "firebrick",                "#B22222" },
  { "floralwhite",              "#FFFAF0" },
  { "forestgreen",              "#228B22" },
  { "fuchsia",                  "#FF00FF" },
  { "gainsboro",                "#DCDCDC" },
  { "ghostwhite",               "#F8F8FF" },
  { "gold",                     "#FFD700" },
  { "goldenrod",                "#DAA520" },
  { "gray",                     "#808080" },
  { "grey",                     "#808080" },
  { "green",                    "#008000" },
  { "greenyellow",              "#ADFF2F" },
  { "honeydew",                 "#F0FFF0" },
  { "hotpink",                  "#FF69B4" },
  { "indianred",                "#CD5C5C" },
  { "indigo",                   "#4B0082" },
  { "ivory",                    "#FFFFF0" },
  { "khaki",                    "#F0E68C" },
  { "lavender",                 "#E6E6FA" },
  { "lavenderblush",            "#FFF0F5" },
  { "lawngreen",                "#7CFC00" },
  { "lemonchiffon",             "#FFFACD" },
  { "lightblue",                "#ADD8E6" },
  { "lightcoral",               "#F08080" },
  { "lightcyan",                "#E0FFFF" },
  { "lightgoldenrodyellow",     "#FAFAD2" },
  { "lightgray",                "#D3D3D3" },
  { "lightgrey",                "#D3D3D3" },
  { "lightgreen",               "#90EE90" },
  { "lightpink",                "#FFB6C1" },
  { "lightsalmon",              "#FFA07A" },
  { "lightseagreen",            "#20B2AA" },
  { "lightskyblue",             "#87CEFA" },
  { "lightslategray",           "#778899" },
  { "lightslategrey",           "#778899" },
  { "lightsteelblue",           "#B0C4DE" },
  { "lightyellow",              "#FFFFE0" },
  { "lime",                     "#00FF00" },
  { "limegreen",                "#32CD32" },
  { "linen",                    "#FAF0E6" },
  { "magenta",                  "#FF00FF" },
  { "maroon",                   "#800000" },
  { "mediumaquamarine",         "#66CDAA" },
  { "mediumblue",               "#0000CD" },
  { "mediumorchid",             "#BA55D3" },
  { "mediumpurple",             "#9370DB" },
  { "mediumseagreen",           "#3CB371" },
  { "mediumslateblue",          "#7B68EE" },
  { "mediumspringgreen",        "#00FA9A" },
  { "mediumturquoise",          "#48D1CC" },
  { "mediumvioletred",          "#C71585" },
  { "midnightblue",             "#191970" },
  { "mintcream",                "#F5FFFA" },
  { "mistyrose",                "#FFE4E1" },
  { "moccasin",                 "#FFE4B5" },
  { "navajowhite",              "#FFDEAD" },
  { "navy",                     "#000080" },
  { "oldlace",                  "#FDF5E6" },
  { "olive",                    "#808000" },
  { "olivedrab",                "#6B8E23" },
  { "orange",                   "#FFA500" },
  { "orangered",                "#FF4500" },
  { "orchid",                   "#DA70D6" },
  { "palegoldenrod",            "#EEE8AA" },
  { "palegreen",                "#98FB98" },
  { "paleturquoise",            "#AFEEEE" },
  { "palevioletred",            "#DB7093" },
  { "papayawhip",               "#FFEFD5" },
  { "peachpuff",                "#FFDAB9" },
  { "peru",                     "#CD853F" },
  { "pink",                     "#FFC0CB" },
  { "plum",                     "#DDA0DD" },
  { "powderblue",               "#B0E0E6" },
  { "purple",                   "#800080" },
  { "rebeccapurple",            "#663399" },
  { "red",                      "#FF0000" },
  { "rosybrown",                "#BC8F8F" },
  { "royalblue",                "#4169E1" },
  { "saddlebrown",              "#8B4513" },
  { "salmon",                   "#FA8072" },
  { "sandybrown",               "#F4A460" },
  { "seagreen",                 "#2E8B57" },
  { "seashell",                 "#FFF5EE" },
  { "sienna",                   "#A0522D" },
  { "silver",                   "#C0C0C0" },
  { "skyblue",                  "#87CEEB" },
  { "slateblue",                "#6A5ACD" },
  { "slategray",                "#708090" },
  { "slategrey",                "#708090" },
  { "snow",                     "#FFFAFA" },
  { "springgreen",              "#00FF7F" },
  { "steelblue",                "#4682B4" },
  { "tan",                      "#D2B48C" },
  { "teal",                     "#008080" },
  { "thistle",                  "#D8BFD8" },
  { "tomato",                   "#FF6347" },
  { "turquoise",                "#40E0D0" },
  { "violet",                   "#EE82EE" },
  { "wheat",                    "#F5DEB3" },
  { "white",                    "#FFFFFF" },
  { "whitesmoke",               "#F5F5F5" },
  { "yellow",                   "#FFFF00" },
  { "yellowgreen",              "#9ACD32" }
};

//-----------------------------------------------------------------------
// Construct RGB colour from string
RGB::RGB(const string& s)
{
  string ss = Text::tolower(s);

  // Remove # if present
  if (!ss.empty() && ss[0] == '#') ss.erase(0,1);

  size_t len = ss.size();

  // Is the rest all hex?
  if (ss.find_first_not_of("0123456789abcdef") == string::npos)
  {
    switch (len)
    {
      case 3:
      {
        unsigned int n = Text::xtoi(ss);
        // Double up nybbles - e.g. f -> 0xff
        r = (((n >> 4) & 0xf0) | ((n >> 8) & 0x0f)) / 255.0;
        g = (((n >> 0) & 0xf0) | ((n >> 4) & 0x0f)) / 255.0;
        b = (((n << 4) & 0xf0) | ((n >> 0) & 0x0f)) / 255.0;
        return;  // Done
      }

      case 6:
      {
        uint64_t n = Text::xtoi64(ss);
        r = ((n >> 16) & 255) / 255.0;
        g = ((n >> 8)  & 255) / 255.0;
        b = ((n     )  & 255) / 255.0;
        return;  // Done
      }

      // Otherwise fall through to other possibilities
    }
  }

  // Check for colour name
  const auto it = colour_names.find(ss);
  if (it != colour_names.end())
  {
    *this = RGB(it->second);  // Construct from #rrggbb and copy
    return;
  }

  // Check for rgb(r,g,b), either float or 0..255,255,255
  if (len >= 10 && string(ss, 0, 4) == "rgb(")
  {
    string params(ss, 4, len-5);  // Remove rgb( and )
    vector<string> fields = Text::split(params, ',');
    if (fields.size() == 3)
    {
      r = Text::stof(fields[0]);
      g = Text::stof(fields[1]);
      b = Text::stof(fields[2]);

      // If more than 1.0, it'll be an integer 0..255
      if (r > 1.0) r/=255.0;
      if (g > 1.0) g/=255.0;
      if (b > 1.0) b/=255.0;
      return;
    }
  }

  // Check for hsl(h,s,l), either float or 0..360,100,100
  if (len >= 10 && string(ss, 0, 4) == "hsl(")
  {
    string params(ss, 4, len-5);  // Remove hsl( and )
    vector<string> fields = Text::split(params, ',');
    if (fields.size() == 3)
    {
      intens_t h = Text::stof(fields[0]);
      intens_t s = Text::stof(fields[1]);
      intens_t l = Text::stof(fields[2]);

      // If more than 1.0, it'll be an integer 0..360 for h, 100 for s,l
      if (h > 1.0) h/=360.0;
      if (s > 1.0) s/=100.0;
      if (l > 1.0) l/=100.0;

      // Convert from HSL to RGB and copy
      *this = RGB(HSL(h,s,l));
      return;
    }
  }

  throw(runtime_error("Unrecognised colour format "+s));
}

//-----------------------------------------------------------------------
// Construct RGB colour from HSL
// Adapted from programmingalgorithms.com

// Helper
static intens_t hue_to_rgb(intens_t v1, intens_t v2, intens_t vh)
{
  if (vh < 0) vh += 1;
  if (vh > 1) vh -= 1;
  if (6*vh < 1) return v1+(v2-v1)*6*vh;
  if (2*vh < 1) return v2;
  if (3*vh < 2) return v1+(v2-v1)*(2.0/3-vh)*6;
  return v1;
}

RGB::RGB(const HSL& hsl)
{
  if (!hsl.s)
    r = g = b = hsl.l;
  else
  {
    intens_t v2 = hsl.l<0.5 ? hsl.l*(1+hsl.s) : (hsl.l+hsl.s)-(hsl.l*hsl.s);
    intens_t v1 = 2*hsl.l - v2;
    r = hue_to_rgb(v1, v2, hsl.h + 1.0/3);
    g = hue_to_rgb(v1, v2, hsl.h);
    b = hue_to_rgb(v1, v2, hsl.h - 1.0/3);
  }
}

//--------------------------------------------------------------------------
// Get as a string
string RGB::str() const
{
  auto s = stringstream{};
  s << "#" << uppercase
    << setfill('0') << setw(2) << hex << static_cast<int>(r*255)
    << setfill('0') << setw(2) << hex << static_cast<int>(g*255)
    << setfill('0') << setw(2) << hex << static_cast<int>(b*255);
  return s.str();
}

//-----------------------------------------------------------------------
// Construct HSL colour from RGB
// Adapted from programmingalgorithms.com
HSL::HSL(const RGB& rgb)
{
  intens_t cmin = min(min(rgb.r, rgb.g), rgb.b);
  intens_t cmax = max(max(rgb.r, rgb.g), rgb.b);
  float delta = cmax - cmin;

  l = (cmax + cmin)/2;

  if (!delta)  // all equal => greyscale
  {
    h = 0.0;
    s = 0.0;
  }
  else
  {
    s = l <= 0.5 ? delta/(cmax+cmin) : delta/(2-cmax-cmin);

    if (rgb.r == cmax)
      h = (rgb.g - rgb.b)/6/delta;
    else if (rgb.g == cmax)
      h = 1.0/3 + (rgb.b - rgb.r)/6/delta;
    else
      h = 2.0/3 + (rgb.r - rgb.g)/6/delta;

    if (h < 0.0) h += 1.0;
    if (h > 1.0) h -= 1.0;
  }
}

}} // namespaces
