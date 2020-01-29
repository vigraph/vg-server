//==========================================================================
// ViGraph dmx graphics modules: dmx-module.h
//
// Common definitions for dmx graphics modules
//
// Copyright (c) 2017-2019 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_DMX_MODULE_H
#define __VIGRAPH_DMX_MODULE_H

#include "../module.h"
#include "vg-dataflow.h"
#include "vg-dmx.h"

//==========================================================================
// JSON conversions
namespace ViGraph { namespace Dataflow {

template<> inline
string get_module_type<DMX::State>() { return "dmx"; }

template<> inline void set_from_json(DMX::State& dmx,
                                     const JSON::Value& json)
{
  dmx.set_from_json(json);
}

template<> inline JSON::Value get_as_json(const DMX::State& dmx)
{
  return dmx.get_as_json();
}

}} //namespaces

#endif // !__VIGRAPH_DMX_MODULE_H
