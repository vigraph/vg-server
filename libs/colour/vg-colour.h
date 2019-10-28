//==========================================================================
// ViGraph vector graphics: vg-colour.h
//
// Definition of colour structures and manipulators
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VG_COLOUR_H
#define __VG_COLOUR_H

#include <string>

// There's an RGB macro in the windows headers :-(
#if defined(PLATFORM_WINDOWS)
#undef RGB
#endif

namespace ViGraph { namespace Colour {

using namespace std;

typedef double intens_t;  // Intensity

struct HSL;  // forward

//==========================================================================
// RGB colour
struct RGB
{
  intens_t r{0.0};
  intens_t g{0.0};
  intens_t b{0.0};

  // Constructors
  RGB() {}
  RGB(intens_t _r, intens_t _g, intens_t _b): r(_r), g(_g), b(_b) {}
  RGB(intens_t _i): r(_i), g(_i), b(_i) {}
  RGB(const string& s);
  RGB(const HSL& hsl);

  // Check for black
  bool is_black() { return !r && !g && !b; }

  // Get greyscale intensity - average r,g,b
  intens_t get_intensity() const { return (r+g+b)/3; }

  // Fade by a proportion, all channels
  void fade(intens_t f) { r*=f; b*=f; g*=f; }

  // Blend between this and another colour
  // frac=0 => all 'this', frac=1 => all 'o'
  RGB blend_with(const RGB& o, double frac)
  {
    double rfrac = 1-frac;
    return RGB(r*rfrac + o.r*frac,
               g*rfrac + o.g*frac,
               b*rfrac + o.b*frac);
  }

  // Equality
  bool operator==(const RGB& o) const
  { return r==o.r && g==o.g && b==o.b; }

  bool operator!=(const RGB& o) const
  { return r!=o.r || g!=o.g || b!=o.b; }

  // Static helpers
  // Create from hex RGB
  static RGB from_rgb_hex(unsigned char r, unsigned char g, unsigned char b)
  { return RGB(r/255.0, g/255.0, b/255.0); }

  // Get as a string
  string str() const;
};

//==========================================================================
// RGB+alpha colour
struct RGBA: public RGB
{
  intens_t a{0.0};  // Transparent

  // Constructors
  RGBA() {}
  RGBA(const RGB& _rgb): RGB(_rgb), a(1.0) {}  // opaque
  RGBA(const RGB& _rgb, intens_t _a): RGB(_rgb), a(_a) {}
  RGBA(intens_t _r, intens_t _g, intens_t _b, intens_t _a):
    RGB(_r, _g, _b), a(_a) {}
  RGBA(intens_t _i, intens_t _a): RGB(_i), a(_a) {}
  // General from string as RGB, but with alpha added
  RGBA(const string& s, intens_t _a): RGB(s), a(_a) {}
  // Specific, #rrggbbaa and #rgba only
  RGBA(const string& s);

  // Tests
  bool is_opaque() { return a == 1.0; }
  bool is_transparent() { return a == 0.0; }

  // Equality
  bool operator==(const RGBA& o) const
  { return RGB::operator==(o) && a==o.a; }

  bool operator!=(const RGBA& o) const
  { return RGB::operator!=(o) || a!=o.a; }

  // Blend between this and another colour
  RGB blend_with(const RGB& o) const
  {
    double oa = 1-a;
    return RGB(r*a + o.r*oa,
               g*a + o.g*oa,
               b*a + o.b*oa);
  }

  // Same, in place
  void blend_over(RGB& o) const
  {
    double oa = 1-a;
    o.r = r*a + o.r*oa,
    o.g = g*a + o.g*oa,
    o.b = b*a + o.b*oa;
  }

  // Get as a string
  string str() const;
};

//==========================================================================
// HSL colour
struct HSL
{
  intens_t h{0.0};   // We store as 0..1 - 1=360
  intens_t s{0.0};
  intens_t l{0.0};

  // Constructors
  HSL() {}
  HSL(intens_t _h, intens_t _s, intens_t _l): h(_h), s(_s), l(_l) {}
  HSL(const RGB& rgb);

  // Equality
  bool operator==(const HSL& o) const
  { return h==o.h && s==o.s && l==o.l; }

  bool operator!=(const HSL& o) const
  { return h!=o.h || s!=o.s || l!=o.l; }
};

//==========================================================================
// Special colours
const RGB black;
const RGB white{1.0};
const RGB red     {1.0, 0.0, 0.0};
const RGB green   {0.0, 1.0, 0.0};
const RGB blue    {0.0, 0.0, 1.0};
const RGB yellow  {1.0, 1.0, 0.0};
const RGB cyan    {0.0, 1.0, 1.0};
const RGB magenta {1.0, 0.0, 1.0};

//==========================================================================
}} //namespaces
#endif // !__VG_COLOUR_H
