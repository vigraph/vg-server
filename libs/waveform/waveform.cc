//==========================================================================
// ViGraph Waveform library: waveform.cc
//
// Waveform functions
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-waveform.h"
#include "vg-geometry.h"

namespace ViGraph { namespace Waveform {

using namespace ViGraph::Geometry;

//==========================================================================
// Get waveform name
string get_name(Type wf)
{
  switch (wf)
  {
    case Type::none:
      return "none";
    case Type::saw:
      return "saw";
    case Type::sin:
      return "sin";
    case Type::square:
      return "square";
    case Type::triangle:
      return "triangle";
    case Type::random:
      return "random";
  }
}

//==========================================================================
// List of type names
set<string> get_names()
{
  return {"none", "saw", "sin", "square", "triangle", "random"};
}

//==========================================================================
// Get waveform type
bool get_type(const string& str, Type& wf)
{
  if (str.empty() || str == "none")
    wf = Type::none;
  else if (str == "saw")
    wf = Type::saw;
  else if (str == "sin")
    wf = Type::sin;
  else if (str == "square")
    wf = Type::square;
  else if (str == "triangle")
    wf = Type::triangle;
  else if (str == "random")
    wf = Type::random;
  else
  {
    wf = Type::none;
    return false;
  }
  return true;
}

//==========================================================================
// Get waveform value (-1..1) for a given type, pulse width and theta
double get_value(Type wf, double pw, double theta)
{
  switch (wf)
  {
    case Type::none:
      return 0;

    case Type::saw:
      if (theta < 0.5)
        return theta * 2;
      else
        return theta * 2 - 2;

    case Type::sin:
      // !!! TODO: can this be improved in terms of pulse width?
      return theta < pw ? sin(theta * pi / pw)
                        : sin((theta - 1.0) * pi / (1.0 - pw));

    case Type::square:
      return (theta < pw) ? 1.0 : -1.0;

    case Type::triangle:
      if (theta < pw / 2)
        return theta / (pw / 2);
      else if (theta >= (1 - (pw / 2)))
        return (theta - (1 - (pw / 2))) / (pw / 2) - 1.0;
      else
        return 1 - (theta - (pw / 2)) / ((1 - pw) / 2);

    case Type::random:
      return 2.0 * rand() / RAND_MAX - 1;
  }
}

}} //namespaces
