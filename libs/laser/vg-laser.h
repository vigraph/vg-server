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
 public:
  //-----------------------------------------------------------------------
  // Constructor
  Optimiser() {}

  //-----------------------------------------------------------------------
  // Add repeated points as blanking anchors
  vector<Point> add_blanking_anchors(const vector<Point>& points,
                                     int leading, int trailing);


  //-----------------------------------------------------------------------
  // Add repeated points at vertices
  vector<Point> add_vertex_repeats(const vector<Point>& points,
                                   double max_angle, // radians
                                   int repeats);

  //-----------------------------------------------------------------------
  // Infill points to enforce a maximum distance
  vector<Point> infill_lines(const vector<Point>& points,
                             double max_distance);

  //-----------------------------------------------------------------------
  // Reorder segments of points to find optimal path
  vector<Point> reorder_segments(const vector<Point>& points);
};

//==========================================================================
}} //namespaces
#endif // !__VG_LASER_H
