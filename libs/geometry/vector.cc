//==========================================================================
// ViGraph vector library: vector.cc
//
// Implementation of vectors
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-geometry.h"

namespace ViGraph { namespace Geometry {

std::ostream& operator<<(std::ostream& s, const Vector& v)
{
  s << '(' << v.x << ',' << v.y;
  if (v.z) s << ',' << v.z;
  s << ')';
  return s;
}

}} // namespaces
