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
  void set(channel_t ch, value_t value, bool htp=false)
  {
    // Look for existing or adjacent coverage
    for(auto& rit: regions)
    {
      auto start = rit.first;
      auto& channels = rit.second;
      auto end = start + channels.size();
      if (ch >= start)
      {
        if (ch < end)
        {
          // Existing value
          auto& existing = channels[ch-start];
          if (!htp || value > existing)
            existing = value;
          return;
        }
        else if (ch == end)
        {
          // Adjacent next channel - just add
          channels.push_back(value);
          return;
        }
      }
    }

    // Not found in or adjacent to any existing regions - create a new one
    regions[ch].push_back(value);
  }

  // Combine with another one
  DMXState& operator+=(const DMXState& o)
  {
    // Apply every channel in 'o' to ourselves
    for(const auto& rit: o.regions)
    {
      auto start = rit.first;
      for(auto v: rit.second)
        set(start++, v, true);
    }
    return *this;
  }

  // Set from JSON
  void set_from_json(const JSON::Value& json)
  {
    if (json.type == JSON::Value::ARRAY)
    {
      for(const JSON::Value& jr: json.a)
      {
        if (jr.type != JSON::Value::OBJECT) continue;
        auto start = jr["start"].as_int();
        auto& region = regions[start];
        const auto& values = jr["values"];
        if (values.type != JSON::Value::ARRAY) continue;
        for(const auto& v: values.a)
          region.push_back(v.as_int());
      }
    }
  }

  // Get as JSON
  JSON::Value get_as_json() const
  {
    JSON::Value value{JSON::Value::ARRAY};
    for(const auto& rit: regions)
    {
      auto& rj = value.add(JSON::Value::OBJECT);
      rj.put("start", rit.first);
      auto& chans = rj.put("values", JSON::Value::ARRAY);
      for(auto v: rit.second)
        chans.add(v);
    }
    return value;
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
  dmx.set_from_json(json);
}

template<> inline JSON::Value get_as_json(const DMXState& dmx)
{
  return dmx.get_as_json();
}

}} //namespaces

#endif // !__VIGRAPH_DMX_MODULE_H
