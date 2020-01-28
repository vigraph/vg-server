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
  using channel_t = uint32_t; // Universe*512 + chan
                              // (Universe actually Art-Net port address)
  using value_t = uint8_t;
  map<channel_t, vector<value_t> > regions; // Non-overlapping contiguous
                                            // regions by start channel

  // Set an individual channel, optionally applying Highest Takes Precedence
  // (HTP) to existing value
  void set(channel_t ch, value_t value, bool htp=false);

  // Combine with another one
  DMXState& operator+=(const DMXState& o);

  // Set from JSON
  void set_from_json(const JSON::Value& json);

  // Get as JSON
  JSON::Value get_as_json() const;
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
  dmx.set_from_json(json);
}

template<> inline JSON::Value get_as_json(const DMXState& dmx)
{
  return dmx.get_as_json();
}

}} //namespaces

#endif // !__VIGRAPH_DMX_MODULE_H
