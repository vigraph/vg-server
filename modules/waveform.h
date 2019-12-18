//==========================================================================
// ViGraph modules: waveform.h
//
// Common definitions for waveform type
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_MODULE_WAVEFORM_H
#define __VIGRAPH_MODULE_WAVEFORM_H

#include "vg-waveform.h"

namespace ViGraph { namespace Dataflow {

template<> inline
string get_module_type<Waveform::Type>() { return "waveform"; }

template<> inline void set_from_json(Waveform::Type& value,
                                     const JSON::Value& json)
{
  Waveform::get_type(json.s, value);
}

template<> inline JSON::Value get_as_json(const Waveform::Type& value)
{
  return {get_name(value)};
}


//==========================================================================
}} //namespaces

#endif // !__VIGRAPH_MODULE_WAVEFORM_H
