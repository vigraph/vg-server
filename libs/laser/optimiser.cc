//==========================================================================
// ViGraph laser graphics library: optimiser.cc
//
// Frame optimisation for laser graphics
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-laser.h"
#include <map>
#include <list>

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
      if (angle > pi) angle-=2*pi;  // Fix direction
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
                                      coord_t max_distance_lit,
                                      coord_t max_distance_blanked)
{
  Point last_point;
  bool last_point_valid{false};
  vector<Point> new_points;

  for(const auto& p: points)
  {
    // Maximum distance in-fills
    if (last_point_valid)
    {
      // Check point against last
      coord_t d = last_point.distance_to(p);
      coord_t max_distance = p.is_lit()?max_distance_lit:max_distance_blanked;
      if (max_distance && d > max_distance)
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

  // If we are filling blanked lines, do the flyback to the start point as
  // well
  if (max_distance_blanked && last_point_valid)
  {
    const auto& p = points[0];
    coord_t d = last_point.distance_to(p);
    if (d > max_distance_blanked)
    {
      Point p0(last_point, Colour::black);  // blanked
      Line l(p0, p);
      coord_t interval = 1.0/ceil(d/max_distance_blanked);
      for(coord_t t=interval; t<1.0-interval/2; t+=interval)
        new_points.emplace_back(l.interpolate(t));
    }
  }

  return new_points;
}

//-----------------------------------------------------------------------
// Reorder segments of points to find optimal path
vector<Point> Optimiser::reorder_segments(const vector<Point>& points)
{
  if (points.empty()) return points;

  // Find segments delimited by blanks
  map<int, int> segments;  // start index (blank) -> end index (last lit, excl)
  Point last_point(points.back(), Colour::white);
  int i=0;
  for(const auto& p: points)
  {
    if (p.is_blanked())
    {
      // Note also on first point, when last_point will be end, lit
      if (last_point.is_lit())
      {
        // Segment starts here
        // Close last segment (if any)
        if (!segments.empty()) segments.rbegin()->second = i-1;

        // Open new one
        segments[i] = -1;
      }
    }
    last_point = p;
    i++;
  }

  // Close off last
  if (segments.empty()) return points;
  segments.rbegin()->second = i-1;

  // Get segments in 'best' order - by naive method of just finding closest
  // one to each one in turn
  list<pair<int, int>> ordered_segments;

  // Take the first one as starting point
  auto current_segment = segments.begin();
  ordered_segments.emplace_back(*current_segment);
  last_point = points[current_segment->first];
  segments.erase(current_segment);

  // Loop through all the segments - this is O(N^2)!
  while (!segments.empty())
  {
    // Find the nearest start to the last point of those remaining
    auto best_segment = segments.end();
    coord_t best_d = coord_max_range;
    for(auto it = segments.begin(); it!=segments.end(); ++it)
    {
      const Point& start_point = points[it->first];
      coord_t d = last_point.distance_to(start_point);
      if (d < best_d)
      {
        best_segment = it;
        best_d = d;
      }
    }

    // Use this one and erase it
    ordered_segments.emplace_back(*best_segment);
    last_point = points[best_segment->first];
    segments.erase(best_segment);
  }

  // Copy points out in order
  vector<Point> new_points;
  for(const auto& it: ordered_segments)
    new_points.insert(new_points.end(), points.begin()+it.first,
                      points.begin()+it.second+1);

  return new_points;
}

//-----------------------------------------------------------------------
// Strip out long runs of blanks (longer than threshold)
vector<Point> Optimiser::strip_blank_runs(const vector<Point>& points,
                                          int threshold)
{
  int blanks{0};
  vector<Point> new_points;

  for(const auto& p: points)
  {
    if (p.is_blanked())
      blanks++;
    else
      blanks=0;

    // Only copy if lit or blanks less than threshold
    if (blanks <= threshold)
      new_points.emplace_back(p);
  }

  return new_points;
}

}} // namespaces
