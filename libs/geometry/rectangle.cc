//==========================================================================
// ViGraph vector library: rectangle.cc
//
// Implementation of rectangles
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-geometry.h"
#include <cmath>

namespace ViGraph { namespace Geometry {

// Normalise so p0 < p1
void Rectangle::normalise()
{
  if (p0.x > p1.x) std::swap(p0.x, p1.x);
  if (p0.y > p1.y) std::swap(p0.y, p1.y);
  if (p0.z > p1.z) std::swap(p0.z, p1.z);
}

// Check if it contains a point
bool Rectangle::contains(const Point& p) const
{
  if (p.x < p0.x || p.x > p1.x) return false;
  if (p.y < p0.y || p.y > p1.y) return false;
  if (p.z < p0.z || p.z > p1.z) return false;
  return true;
}

// Check if it overlaps another rectangle
// Touching is considered overlap
bool Rectangle::overlaps(const Rectangle& r) const
{
  // Overlap if we contain either of their points or they contain
  // either of ours
  return contains(r.p0) || contains(r.p1) || r.contains(p0) || r.contains(p1);
}

// Expand to include a point
void Rectangle::expand_to_include(const Point& p)
{
  if (p.x < p0.x) p0.x = p.x;
  else if (p.x > p1.x) p1.x = p.x;

  if (p.y < p0.y) p0.y = p.y;
  else if (p.y > p1.y) p1.y = p.y;

  if (p.z < p0.z) p0.z = p.z;
  else if (p.z > p1.z) p1.z = p.z;
}

// Become the bounding box of a range of points
void Rectangle::become_bounding_box(std::vector<Point>::const_iterator begin,
                                    std::vector<Point>::const_iterator end)
{
  bool first = true;
  for(auto it=begin; it!=end; it++)
  {
    if (first)
    {
      p0 = p1 = *it;  // Special, reset to first point
      first = false;
    }
    else expand_to_include(*it);
  }
}

// Output operator
std::ostream& operator<<(std::ostream& s, const Rectangle& r)
{
  s << r.p0 << "-" << r.p1;
  return s;
}

}} // namespaces
