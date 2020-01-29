//==========================================================================
// ViGraph DMX library: state.cc
//
// State structure for DMX
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-dmx.h"

namespace ViGraph { namespace DMX {

//--------------------------------------------------------------------------
// Set an individual channel, optionally applying Highest Takes Precedence
// (HTP) to existing value
void State::set(channel_t ch, value_t value, bool htp)
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

//--------------------------------------------------------------------------
// Get value of an individual channel, or def if not set
value_t State::get(channel_t ch, value_t def) const
{
  for(auto& rit: regions)
  {
    auto start = rit.first;
    // Stop if passed
    if (ch < start) return def;

    auto& channels = rit.second;
    auto end = start + channels.size();
    if (ch < end) return channels[ch-start];
  }

  return def;
}

//--------------------------------------------------------------------------
// Flatten to a set of full universe buffers
void State::flatten(map<int, UniverseData>& universes) const
{
  for(const auto& rit: regions)
  {
    auto chan = rit.first;
    for(auto v: rit.second)
    {
      auto u = chan / DMX::channels_per_universe;
      auto& ud = universes[u];
      ud.channels[chan % DMX::channels_per_universe] = v;
      chan++;
    }
  }
}

//--------------------------------------------------------------------------
// Combine with another one with HTP
State& State::operator+=(const State& o)
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

//--------------------------------------------------------------------------
// Set from JSON
void State::set_from_json(const JSON::Value& json)
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

//--------------------------------------------------------------------------
// Get as JSON
JSON::Value State::get_as_json() const
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

}} // namespaces
