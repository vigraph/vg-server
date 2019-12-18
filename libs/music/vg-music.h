//==========================================================================
// ViGraph music: vg-music.h
//
// Music theory library
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VG_MUSIC_H
#define __VG_MUSIC_H

#include <cmath>

namespace ViGraph { namespace Music {

// Make our lives easier without polluting anyone else
using namespace std;

const auto concert_a4_frequency = 440.0;

//==========================================================================
// CV (1.0 per octave, C4=0) to frequency
double cv_to_frequency(double cv)
{
  return concert_a4_frequency * pow(2.0, cv-0.75);  // A4 = C4+9/12 = CV 0.75
}

//==========================================================================
}} //namespaces
#endif // !__VG_MUSIC_H
