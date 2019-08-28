//==========================================================================
// ViGraph colour library: rgba.cc
//
// Implementation of RGB-Alpha colour constructors & conversions
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-colour.h"
#include "ot-text.h"
#include <stdexcept>
#include <map>
#include <sstream>
#include <iomanip>

namespace ViGraph { namespace Colour {

using namespace ObTools;

//-----------------------------------------------------------------------
// Construct RGB-Alpha colour from string - note we only support
// #rrggbbaa and #rgba format, rest is done with RGB(string) and separate
// alpha
RGBA::RGBA(const string& s)
{
  string ss = Text::tolower(s);

  // Remove # if present
  if (!ss.empty() && ss[0] == '#') ss.erase(0,1);

  size_t len = ss.size();

  // Is the rest all hex?
  if (ss.find_first_not_of("0123456789abcdef") == string::npos)
  {
    switch (len)
    {
      case 4:
      {
        unsigned int n = Text::xtoi(ss);
        // Double up nybbles - e.g. f -> 0xff
        r = (((n >> 8) & 0xf0) | ((n >> 12) & 0x0f)) / 255.0;
        g = (((n >> 4) & 0xf0) | ((n >> 8) & 0x0f)) / 255.0;
        b = (((n >> 0) & 0xf0) | ((n >> 4) & 0x0f)) / 255.0;
        a = (((n << 4) & 0xf0) | ((n >> 0) & 0x0f)) / 255.0;
        return;  // Done
      }

      case 8:
      {
        uint64_t n = Text::xtoi64(ss);
        r = ((n >> 24) & 255) / 255.0;
        g = ((n >> 16) & 255) / 255.0;
        b = ((n >>  8) & 255) / 255.0;
        a = ((n      ) & 255) / 255.0;
        return;  // Done
      }
    }
  }

  throw(runtime_error("Unrecognised colour format "+s));
}

//--------------------------------------------------------------------------
// Get as a string
string RGBA::str() const
{
  auto s = stringstream{};
  s << RGB::str() << uppercase
    << setfill('0') << setw(2) << hex << static_cast<int>(a*255);
  return s.str();
}

}} // namespaces
