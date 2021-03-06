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
  int width{0};                 // Safe default
  vector<Colour::PackedRGBA> pixels;  // In raster order

public:
  // Constructors
  Rectangle() {}
  Rectangle(int _width, int _height): width(_width), pixels(_width*_height) {}
  Rectangle(const string& ppm) { read_from_ppm(ppm); }

  // Accessors
  int get_width() const  { return width; }
  int get_height() const { return width ? pixels.size() / width : 0; }
  Vector size() const { return Vector(width, get_height()); }

  // Resize
  void resize(int _width, int _height)
  {
    width = _width;
    pixels.resize(_width * _height);
  }

  // Pixel access
  vector<Colour::PackedRGBA>& get_pixels() { return pixels; }
  Colour::RGBA get(int x, int y) const
  { return pixels[y*width+x].unpack(); }
  Colour::RGBA operator()(int x, int y) const
  { return get(x,y); }

  void set(int x, int y, const Colour::RGBA& c)
  { pixels[y*width+x] = c; }

  // Fill to a colour
  void fill(const Colour::RGBA& c)
  { Colour::PackedRGBA pc(c); for(auto& p: pixels) p=pc; }

  // Set all pixels to a colour, maintaining existing alpha
  void colourise(const Colour::RGB& c)
  { for(auto& p: pixels) p.colourise(c); }

  // Fade to an alpha (combines with existing)
  void fade(double alpha)
  { for(auto& p: pixels) p.fade(alpha); }

  // Fill a set of polygons
  // Closes polygons demarcated by blanked points, colour from final point
  void fill_polygons(const vector<Geometry::Point>& points);

  // Blit (copy) into the given (x,y) position in a destination rectangle
  // Copies overwriting dest, including alpha
  // Clips to destination rectangle
  void blit(const Vector& pos, Rectangle& dest) const;

  // Apply in the given (x,y) position in a destination rectangle
  // Blends with alpha over dest, retaining dest's alpha
  // Clips to destination rectangle
  void apply(const Vector& pos, Rectangle& dest) const;

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
    Vector pos;  // Defines *centre* of rectangle, in vg-geometry space
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

  // Get the bounding box of all items
  Geometry::Rectangle bounding_box() const;

  // Flatten into a single Rectangle
  // Individual bitmaps will be clipped to the result's size
  void compose(Rectangle& result) const;

  // Combine with another one
  Group& operator+=(const Group& o)
  { items.insert(items.end(), o.items.begin(), o.items.end()); return *this; }
};

//==========================================================================
}} //namespaces
#endif // !__VG_BITMAP_H
