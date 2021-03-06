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
  RGB blend_with(const RGB& o, double frac) const
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

  // Combine with another one, additive, fencing at 1.0
  RGB& operator+=(const RGB& o)
  { r+=o.r; r=min(1.0, r);
    g+=o.g; g=min(1.0, g);
    b+=o.b; b=min(1.0, b);
    return *this; }

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
  bool is_opaque() const { return a == 1.0; }
  bool is_transparent() const { return a == 0.0; }

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
// RGB+alpha colour packed into 32-bit as RGBA 8888, R is LSB, A is MSB
struct PackedRGBA
{
  using packed_t=uint32_t;
  packed_t packed;

  // Constructors
  PackedRGBA(): packed(0) {}
  PackedRGBA(packed_t p): packed(p) {}
  PackedRGBA(const RGBA& c)
  {
    packed =  static_cast<packed_t>(c.r * 255.0)
           | (static_cast<packed_t>(c.g * 255.0) << 8)
           | (static_cast<packed_t>(c.b * 255.0) << 16)
           | (static_cast<packed_t>(c.a * 255.0) << 24);
  }

  // Unpack to RGBA
  RGBA unpack() const
  {
    return RGBA(static_cast<intens_t>( packed        & 0xff)/255.0,
                static_cast<intens_t>((packed >>  8) & 0xff)/255.0,
                static_cast<intens_t>((packed >> 16) & 0xff)/255.0,
                static_cast<intens_t>((packed >> 24) & 0xff)/255.0);
  }

  // Equality
  bool operator==(const PackedRGBA& o) const
  { return packed==o.packed; }

  bool operator!=(const PackedRGBA& o) const
  { return packed!=o.packed; }

  // Get byte values of each channel
  uint8_t r8() const { return static_cast<uint8_t>(packed & 0xff); }
  uint8_t g8() const { return static_cast<uint8_t>((packed >> 8) & 0xff); }
  uint8_t b8() const { return static_cast<uint8_t>((packed >> 16) & 0xff); }
  uint8_t a8() const { return static_cast<uint8_t>((packed >> 24) & 0xff); }

  // Tests
  bool is_opaque() const { return packed >= 0xFF000000ul; }
  bool is_transparent() const { return packed < 0x01000000ul; }

  // Blend between this and another colour
  PackedRGBA blend_with(const PackedRGBA& o) const
  {
    // Quick wins
    if (is_transparent()) return o;
    if (is_opaque()) return *this;

    // Unpack, blend and repack
    return PackedRGBA(unpack().blend_with(o.unpack()));
  }

  // Same, in place
  void blend_over(PackedRGBA& o) const
  {
    // Quick wins
    if (is_transparent()) { return; }
    if (is_opaque()) { o=*this; return; }

    // Slow train
    o = PackedRGBA(unpack().blend_with(o.unpack()));
  }

  // Fade with an alpha value - combines with existing alpha
  void fade(intens_t alpha)
  {
    auto old_alpha = static_cast<intens_t>(a8())/255.0;
    alpha *= old_alpha;
    packed = (packed & 0xFFFFFF) | (static_cast<packed_t>(alpha * 255.0) << 24);
  }

  // Colourise - set colour, keeping existing alpha
  void colourise(const RGB& c)
  {
    PackedRGBA pc(c);
    packed &= 0xFF000000ul;                // strip colour, keeping alpha
    packed |= (pc.packed & 0x00FFFFFFul);  // add back
  }
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
