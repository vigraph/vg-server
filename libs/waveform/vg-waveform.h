//==========================================================================
// ViGraph waveform: vg-waveform.h
//
// Waveform library
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VG_WAVEFORM_H
#define __VG_WAVEFORM_H

#include <string>
#include <set>

namespace ViGraph { namespace Waveform {

// Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Waveform types
enum class Type
{
  none,
  saw,
  sin,
  square,
  triangle,
  random
};

//==========================================================================
// List of type names
set<string> get_names();

//==========================================================================
// Get waveform name
string get_name(Type wf);

//==========================================================================
// Get waveform type
bool get_type(const string& str, Type& wf);

//==========================================================================
// Get waveform value (0..1) for a given type, pulse width and theta
double get_value(Type wf, double width, double theta);

//==========================================================================
}} //namespaces
#endif // !__VG_WAVEFORM_H
