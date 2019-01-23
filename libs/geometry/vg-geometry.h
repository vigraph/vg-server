//==========================================================================
// ViGraph vector graphics: vg-geometry.h
//
// Definition of core vector graphic concepts - points, vectors etc.
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VG_GEOMETRY_H
#define __VG_GEOMETRY_H

#include "vg-colour.h"
#include <ostream>
#include <math.h>
#include <limits>
#include <vector>

namespace ViGraph { namespace Geometry {

typedef double coord_t;   // X,Y,Z, origin centre, positive right, up, towards
                          // (-0.5, 0.5) inclusive

// Min and max within our standard field
static const coord_t coord_min = -0.5;
static const coord_t coord_max = 0.5;

// Min and max possible to store
static const coord_t coord_min_range = std::numeric_limits<double>::lowest();
static const coord_t coord_max_range = std::numeric_limits<double>::max();

static const double pi = acos(-1);

// -------------------------------------------------------------------------
// Basics

// Vector (pure maths)
struct Vector
{
  coord_t x{0};
  coord_t y{0};
  coord_t z{0};

  // default, for initialisation
  Vector() {}
  // - 2D:
  Vector(coord_t _x, coord_t _y): x(_x), y(_y) {}
  // - 3D:
  Vector(coord_t _x, coord_t _y, coord_t _z):
    x(_x), y(_y), z(_z) {}

  // Vector arithmetic
  Vector operator+(const Vector& o) const { return Vector(x+o.x,y+o.y,z+o.z); }
  Vector& operator+=(const Vector& o) { x+=o.x; y+=o.y; z+=o.z; return *this; }
  Vector operator-(const Vector& o) const { return Vector(x-o.x,y-o.y,z-o.z); }
  Vector& operator-=(const Vector& o) { x-=o.x; y-=o.y; z-=o.z; return *this; }

  // Scale
  Vector operator*(coord_t f) const { return Vector(x*f, y*f, z*f); }
  Vector& operator*=(coord_t f) { x*=f; y*=f; z*=f; return *this; }
  Vector operator/(coord_t f) const { return Vector(x/f, y/f, z/f); }
  Vector& operator/=(coord_t f) { x/=f; y/=f; z/=f; return *this; }

  // Reverse (unary -)
  Vector operator-() const { return Vector(-x, -y, -z); }

  // Equality
  bool operator==(const Vector& o) const
  { return x==o.x && y==o.y && z==o.z; }

  bool operator!=(const Vector& o) const
  { return x!=o.x || y!=o.y || z!=o.z; }

  // Length
  coord_t length() const { return sqrt(x*x + y*y + z*z); }

  // Dot product
  coord_t dot(const Vector& o) { return x*o.x + y*o.y + z*o.z; }

  // Cross product
  Vector cross(const Vector& o)
  { return Vector(y*o.z-z*o.y, z*o.x-x*o.z, x*o.y-y*o.x); }

  // Angle between vectors (radians, counter-clockwise in 2D)
  coord_t angle_to(const Vector& o)
  { return (o.z == z)
      ?atan2(o.y,o.x)-atan2(y,x)         // 2d solution with sign
      :atan2(cross(o).length(), dot(o)); // 3d solution missing sign
  }
};

// >> operator to write to ostream
std::ostream& operator<<(std::ostream& s, const Vector& v);

// Single point, with colour
struct Point: public Vector
{
  Colour::RGB c;

  // default, for initialisation
  Point() {}

  // From vector
  Point(const Vector& _v): Vector(_v) {}
  Point(const Vector& _v, const Colour::RGB& _c): Vector(_v), c(_c) {}

  // Vector constructors for blanked points
  using Vector::Vector;

  // - 2D with colour:
  Point(coord_t _x, coord_t _y, const Colour::RGB& _c):
    Vector(_x, _y), c(_c) {}
  // - 3D lit:
  Point(coord_t _x, coord_t _y, coord_t _z, const Colour::RGB& _c):
    Vector(_x, _y, _z), c(_c) {}

  // Helpers
  bool is_lit() const     { return c.r>0.0 || c.g>0.0 || c.b>0.0; }
  bool is_blanked() const { return c.r==0.0 && c.g==0.0 && c.b==0.0; }
  void blank()            { c = Colour::black; }

  // Distance to another point
  coord_t distance_to(const Point& o) const { return (o-*this).length(); }
};

// Line
struct Line
{
  Point p0;
  Point p1;

  // Default
  Line() {}

  // Two points
  Line(const Point& _p0, const Point& _p1): p0(_p0), p1(_p1) {}

  // Linear interpolation, t=[0..1]
  Point interpolate(coord_t t) const { return Point(p0*(1-t)+p1*t, p0.c); }
};

// -------------------------------------------------------------------------
// Bezier curves
struct QuadraticBezier
{
  Point p0, p1, p2;

  QuadraticBezier(const Point& _p0, const Point& _p1, const Point& _p2):
   p0(_p0), p1(_p1), p2(_p2) {}

  // Interpolation along the curve, t=[0..1]
  Point interpolate(coord_t t) const;
};

struct CubicBezier
{
  Point p0, p1, p2, p3;

  CubicBezier(const Point& _p0, const Point& _p1, const Point& _p2,
              const Point& _p3): p0(_p0), p1(_p1), p2(_p2), p3(_p3) {}

  // Interpolation along the curve, t=[0..1]
  Point interpolate(coord_t t) const;
};

// -------------------------------------------------------------------------
// Rectangle
// (actually a rectangular prism in 3-space)
struct Rectangle
{
  Point p0, p1;

  Rectangle() {}
  Rectangle(const Point& _p0, const Point& _p1):
    p0(_p0), p1(_p1) { normalise(); }

  // Normalise so p0 < p1
  void normalise();

  // Get size as a Vector (only when normalised)
  Vector size() { return p1-p0; }

  // Check if it contains a point (only when normalised)
  bool contains(const Point& p) const;

  // Check if it overlaps another rectangle
  // Touching is considered overlap.  Both must be normalised
  bool overlaps(const Rectangle& r) const;

  // Expand to include a point
  void expand_to_include(const Point& p);

  // Become the bounding box of a vector of Points
  void become_bounding_box(std::vector<Point>& points);
};

std::ostream& operator<<(std::ostream& s, const Rectangle& r);

//==========================================================================
}} //namespaces
#endif // !__VG_GEOMETRY_H
