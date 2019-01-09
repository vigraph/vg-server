//==========================================================================
// ViGraph vector graphics: vg-laser.h
//
// Support library for laser graphics operations
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VG_LASER_H
#define __VG_LASER_H

#include <vector>
#include <string>
#include "vg-geometry.h"

namespace ViGraph { namespace Laser {

// Make our lives easier without polluting anyone else
using namespace std;
using namespace ViGraph;
using namespace ViGraph::Geometry;

//==========================================================================
// Laser frame optimiser
class Optimiser
{
  coord_t max_distance{0.0};
  double max_angle{-1.0};    // in radians
  int vertex_repeats{0};
  int blanking_repeats{0};

 public:
  //-----------------------------------------------------------------------
  // Constructor
  Optimiser() {}

  //-----------------------------------------------------------------------
  // Enable point infill
  // Points are added to make them no more than 'max_distance' apart
  void enable_infill(coord_t _max_distance)
  { max_distance = _max_distance; }

  //-----------------------------------------------------------------------
  // Enable vertex anchors
  // Points are added at vertices above a angle
  void enable_vertex_repeats(double _max_angle, int _repeats)
  { max_angle = _max_angle; vertex_repeats = _repeats; }

  //-----------------------------------------------------------------------
  // Enable blanking anchors
  // Points are added at blanking start/end points
  void enable_blanking_repeats(int _repeats)
  { blanking_repeats = _repeats; }

  //-----------------------------------------------------------------------
  // Optimise a set of points
  vector<Point> optimise(const vector<Point>& points);
};

//==========================================================================
// Laser frame reorderer
class Reorderer
{
 public:
  //-----------------------------------------------------------------------
  // Constructor
  Reorderer() {}

  //-----------------------------------------------------------------------
  // Reorder a set of points
  vector<Point> reorder(const vector<Point>& points);
};

//==========================================================================
}} //namespaces
#endif // !__VG_LASER_H
