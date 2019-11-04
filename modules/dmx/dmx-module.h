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

//==========================================================================
// DMX state type
struct DMXState
{
  vector<uint8_t> channels;

  // Combine with another one
  DMXState& operator+=(const DMXState& o)
  {
    // Highest Takes Priority - max of either side
    for(auto i=0u; i<min(channels.size(), o.channels.size()); i++)
      channels[i] = max(channels[i], o.channels[i]);
    return *this;
  }
};

typedef shared_ptr<DMXState> DMXStatePtr;

//==========================================================================
// JSON conversions
namespace ViGraph { namespace Dataflow {

template<> inline
string get_module_type<DMXState>() { return "dmx"; }

template<> inline void set_from_json(DMXState& dmx,
                                     const JSON::Value& json)
{
  if (json.type == JSON::Value::ARRAY)
  {
    for(const JSON::Value& chan: json.a)
      dmx.channels.push_back(chan.as_int());
  }
}

template<> inline JSON::Value get_as_json(const DMXState& dmx)
{
  JSON::Value value{JSON::Value::ARRAY};
  for(const auto& chan: dmx.channels)
    value.add(chan);
  return value;
}

}} //namespaces

#endif // !__VIGRAPH_DMX_MODULE_H
