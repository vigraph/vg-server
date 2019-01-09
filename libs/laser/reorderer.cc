//==========================================================================
// ViGraph laser graphics library: reorderer.cc
//
// Frame reordering for laser graphics
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-laser.h"

namespace ViGraph { namespace Laser {

//-----------------------------------------------------------------------
// Reordering a set of points to find optimal path
vector<Point> Reorderer::reorder(const vector<Point>& points)
{
  vector<Point> new_points;

  for(const auto& p: points)
  {
    new_points.push_back(p);
  }

  return new_points;
}

}} // namespaces
