//==========================================================================
// ViGraph MIDI library: note.cc
//
// MIDI note functions
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-midi.h"
#include "ot-text.h"
#include <cmath>

namespace ViGraph { namespace MIDI {

using namespace ObTools;

//-----------------------------------------------------------------------
// Post a raw MIDI byte into the reader
int get_midi_number(const string& note)
{
  auto s = Text::tolower(Text::remove_space(note));
  if (s.empty())
    return - 1;

  auto m = int{};
  // Get note based in octave 0
  switch (s[0])
  {
    case 'a': m = 21; break;
    case 'b': m = 23; break;
    case 'c': m = 24 - 12; break;
    case 'd': m = 26 - 12; break;
    case 'e': m = 28 - 12; break;
    case 'f': m = 29 - 12; break;
    case 'g': m = 31 - 12; break;
    default: return -1;
  }

  auto o = 4;
  if (s.size() > 1)
  {
    auto opos = 1u;
    // Apply flat or sharp
    switch (s[1])
    {
      case '#': ++m; ++opos; break;
      case 'b': --m; ++opos; break;
      default: break;
    }
    // Get octave
    if (s.size() > opos)
    {
      if (s[opos] >= '0' && s[opos] <= '9')
        o = s[opos] - '0';
    }
  }
  // Apply octave
  m += o * 12;

  return m;
}

//==========================================================================
// MIDI Note frequency
double get_midi_frequency(int number)
{
  return 440.0 * pow(2, (number - 69) / 12.0);
}

}} // namespaces
