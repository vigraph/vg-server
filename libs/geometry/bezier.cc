//==========================================================================
// ViGraph vector library: bezier.cc
//
// Implementation of Bezier curves
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-geometry.h"

namespace ViGraph { namespace Geometry {

// -------------------------------------------------------------------------
// Quadratic Bezier

// Interpolation along the curve, t=[0..1]
Point QuadraticBezier::interpolate(coord_t t) const
{
  Point q0 = Line(p0, p1).interpolate(t);
  Point q1 = Line(p1, p2).interpolate(t);
  return Line(q0, q1).interpolate(t);
}

// -------------------------------------------------------------------------
// Cubic Bezier

// Interpolation along the curve, t=[0..1]
Point CubicBezier::interpolate(coord_t t) const
{
  Point q0 = Line(p0, p1).interpolate(t);
  Point q1 = Line(p1, p2).interpolate(t);
  Point q2 = Line(p2, p3).interpolate(t);
  return QuadraticBezier(q0, q1, q2).interpolate(t);
}

}} // namespaces
