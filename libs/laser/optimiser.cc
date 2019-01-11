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
// Add repeated points as blanking anchors
vector<Point> Optimiser::add_blanking_anchors(const vector<Point>& points,
                                              int leading, int trailing)
{
  Point last_point;
  bool last_point_valid{false};
  vector<Point> new_points;

  for(const auto& p: points)
  {
    if (last_point_valid)
    {
      if (last_point.is_lit() && p.is_blanked())
      {
        // Repeat last lit point
        for(auto i=0; i<trailing; i++)
          new_points.emplace_back(last_point);

        // Repeat this blanked point
        for(auto i=0; i<leading; i++)
          new_points.emplace_back(p);
      }
    }
    else
    {
      // First point - insert leading blanks
      for(auto i=0; i<leading; i++)
        new_points.emplace_back(Point(p));
    }
    new_points.emplace_back(p);

    last_point = p;
    last_point_valid = true;
  }

  // Last point - insert trailing lit
  if (last_point_valid)
  {
    // Repeat last lit point
    for(auto i=0; i<trailing; i++)
      new_points.emplace_back(last_point);
  }

  return new_points;
}

//-----------------------------------------------------------------------
// Add repeated points at vertices
vector<Point> Optimiser::add_vertex_repeats(const vector<Point>& points,
                                            double max_angle, // radians
                                            int repeats)
{
  Point last_point;
  bool last_point_valid{false};
  Vector last_vector;
  bool last_vector_valid{false};
  vector<Point> new_points;

  for(const auto& p: points)
  {
    // Check for vertex with maximum angle
    if (last_vector_valid)
    {
      Vector this_vector = p-last_point;
      coord_t angle = last_vector.angle_to(this_vector);
      if (angle > max_angle || angle < -max_angle)  // Turning either way
      {
        // Repeat point at vertex
        for(auto i=0; i<repeats; i++)
          new_points.emplace_back(last_point);
      }
    }

    new_points.emplace_back(p);

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

//-----------------------------------------------------------------------
// Infill points to enforce a maximum distance
vector<Point> Optimiser::infill_lines(const vector<Point>& points,
                                      double max_distance)
{
  if (!max_distance) return points;

  Point last_point;
  bool last_point_valid{false};
  vector<Point> new_points;

  for(const auto& p: points)
  {
    // Maximum distance in-fills - only for lit lines
    if (last_point_valid && p.is_lit())
    {
      // Check point against last
      coord_t d = last_point.distance_to(p);
      if (d > max_distance)
      {
        // Spread along a line, using new point's colour
        Point p0(last_point, p.c);
        Line l(p0, p);
        // Get first equal interval that satisfies the constraint
        coord_t interval = 1.0/ceil(d/max_distance);
        for(coord_t t=interval; t<1.0-interval/2; t+=interval)
          new_points.emplace_back(l.interpolate(t));
      }
    }

    new_points.emplace_back(p);

    last_point = p;
    last_point_valid = true;
  }

  return new_points;
}

//-----------------------------------------------------------------------
// Reorder segments of points to find optimal path
vector<Point> Optimiser::reorder_segments(const vector<Point>& points)
{
  vector<Point> new_points;

  for(const auto& p: points)
  {
    new_points.push_back(p);
  }

  return new_points;
}

}} // namespaces
