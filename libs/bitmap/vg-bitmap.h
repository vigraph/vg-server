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
#include "vg-geometry.h"

namespace ViGraph { namespace Bitmap {

using namespace std;
using namespace ViGraph::Geometry;

//==========================================================================
// Bitmap rectangle
class Rectangle
{
private:
  int width{1};                 // Safe default
  vector<Colour::RGBA> pixels;  // In raster order

public:
  // Constructors
  Rectangle() {}
  Rectangle(int _width, int _height): width(_width), pixels(_width*_height) {}
  Rectangle(const string& ppm) { read_from_ppm(ppm); }

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

  // Fill to a colour
  void fill(const Colour::RGBA& c)
  { for(auto& p: pixels) p=c; }

  // Fill a set of polygons
  // Closes polygons demarcated by blanked points, colour from final point
  void fill_polygons(vector<Geometry::Point>& points);

  // To/from PPM (P3 ASCII form)
  string to_ppm();
  void read_from_ppm(const string& ppm);

  // Convert to simple ASCII form, for testing
  string to_ascii();
};

//==========================================================================
// Bitmap group - sparse set of bitmaps, with depth order
class Group
{
public:
  struct Item
  {
    Vector pos;
    Rectangle rect;

    Item(const Rectangle& _rect): rect(_rect) {}
    Item(const Vector& _pos, const Rectangle& _rect):
      pos(_pos), rect(_rect) {}
  };

  vector<Item> items;

  // Add a rectangle at 0,0,0
  void add(const Rectangle& rect)
  { items.push_back(Item{rect}); }

  // Add a rectangle at position
  void add(const Vector& pos, const Rectangle& rect)
  { items.push_back(Item{pos, rect}); }
};

//==========================================================================
}} //namespaces
#endif // !__VG_BITMAP_H
