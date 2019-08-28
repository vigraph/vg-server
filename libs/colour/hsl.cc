//==========================================================================
// ViGraph colour library: hsl.cc
//
// Implementation of HSL colour constructors & conversions
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-colour.h"
#include "ot-text.h"

namespace ViGraph { namespace Colour {

using namespace ObTools;

//-----------------------------------------------------------------------
// Construct HSL colour from RGB
// Adapted from programmingalgorithms.com
HSL::HSL(const RGB& rgb)
{
  intens_t cmin = min(min(rgb.r, rgb.g), rgb.b);
  intens_t cmax = max(max(rgb.r, rgb.g), rgb.b);
  float delta = cmax - cmin;

  l = (cmax + cmin)/2;

  if (!delta)  // all equal => greyscale
  {
    h = 0.0;
    s = 0.0;
  }
  else
  {
    s = l <= 0.5 ? delta/(cmax+cmin) : delta/(2-cmax-cmin);

    if (rgb.r == cmax)
      h = (rgb.g - rgb.b)/6/delta;
    else if (rgb.g == cmax)
      h = 1.0/3 + (rgb.b - rgb.r)/6/delta;
    else
      h = 2.0/3 + (rgb.r - rgb.g)/6/delta;

    if (h < 0.0) h += 1.0;
    if (h > 1.0) h -= 1.0;
  }
}

}} // namespaces
