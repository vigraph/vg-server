//==========================================================================
// ViGraph laser graphics library: optimiser.cc
//
// Frame optimisation for laser graphics
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-laser.h"

namespace ViGraph { namespace Laser {

//-----------------------------------------------------------------------
// Optimise a set of points
vector<Point> Optimiser::optimise(const vector<Point>& points)
{
  // Fast null operation
  if (!max_distance && max_angle < 0 && !blanking_repeats)
    return points;

  Point last_point;
  bool last_point_valid{false};
  Vector last_vector;
  bool last_vector_valid{false};
  vector<Point> new_points;

  for(const auto& p: points)
  {
    // Check for vertex with maximum angle
    // Note do this before line infills because we repeat the previous point
    if (max_angle >= 0 && last_vector_valid)
    {
      Vector this_vector = p-last_point;
      coord_t angle = last_vector.angle_to(this_vector);
      if (angle > max_angle || angle < -max_angle)  // Turning either way
      {
        // Repeat point at vertex
        for(auto i=0; i<vertex_repeats; i++)
          new_points.emplace_back(last_point);
      }
    }

    // Maximum distance in-fills - only for lit lines
    if (max_distance && last_point_valid && p.is_lit())
    {
      // Check point against last
      coord_t d = last_point.distance_to(p);
      if (d > max_distance)
      {
        // Spread along a line, using new point's colour
        Point p0(last_point, p.c);
        Line l(p0, p);
        coord_t interval = max_distance / d;
        for(coord_t t=interval; t<=1.0-interval; t+=interval)
          new_points.emplace_back(l.interpolate(t));
      }
    }

    // Blanking in-fills
    if (blanking_repeats)
    {
      if ((last_point_valid
          && ((last_point.is_blanked() && p.is_lit())      // blank to lit
              || (last_point.is_lit() && p.is_blanked()))) // lit to blank
          || !last_point_valid)                            // first point
      {
        // Repeat new point
        for(auto i=0; i<blanking_repeats; i++)
          new_points.emplace_back(p);
      }
    }

    // Always add the point anyway
    new_points.emplace_back(p);

    // State for next time
    if (last_point_valid)
    {
      last_vector = p-last_point;
      last_vector_valid = true;
    }
    last_point = p;
    last_point_valid = true;
  }

  return new_points;
}

}} // namespaces
