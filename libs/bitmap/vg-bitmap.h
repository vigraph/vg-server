//==========================================================================
// ViGraph bitmap graphics: vg-bitmap.h
//
// Definition of bitmap structures and operations
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VG_BITMAP_H
#define __VG_BITMAP_H

#include <string>
#include <vector>
#include "vg-colour.h"

namespace ViGraph { namespace Bitmap {

using namespace std;

//==========================================================================
// Bitmap structure
class Bitmap
{
private:
  int width{1};  // Safe default
  vector<Colour::RGBA> pixels;

public:
  // Constructors
  Bitmap() {}
  Bitmap(int _width, int _height): width(_width), pixels(_width*_height) {}

  // Accessors
  int get_width() const  { return width; }
  int get_height() const { return pixels.size() / width; }

  // Pixel access
  Colour::RGBA get(int x, int y) const
  { return pixels[y*width+x]; }
  Colour::RGBA operator()(int x, int y) const
  { return get(x,y); }

  void set(int x, int y, const Colour::RGBA& c)
  { pixels[y*width+x] = c; }
};


//==========================================================================
}} //namespaces
#endif // !__VG_BITMAP_H
