//==========================================================================
// ViGraph vector graphics: vg-dmx.h
//
// Definitions for DMX support
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VG_DMX_H
#define __VG_DMX_H

#include "ot-json.h"

namespace ViGraph { namespace DMX {

// Make our lives easier without polluting anyone else
using namespace std;
using namespace ViGraph;
using namespace ObTools;

// Typedefs
using channel_t = uint32_t; // Universe*512 + chan
                            // (Universe actually Art-Net port address)
using value_t = uint8_t;

// Constants
static const size_t channels_per_universe{512};
static const value_t max_value{255};

// Virtual channel (0..) from universe (0..) and channel (1..)
inline channel_t channel_number(int u, int c)
{
  return u * channels_per_universe + c - 1;
}

//==========================================================================
// DMX state type
struct State
{
  map<channel_t, vector<value_t> > regions; // Non-overlapping contiguous
                                            // regions by start channel

  // Set an individual channel, optionally applying Highest Takes Precedence
  // (HTP) to existing value
  void set(channel_t ch, value_t value, bool htp=false);

  // Get value of an individual channel, or def if not set
  value_t get(channel_t ch, value_t def=0) const;

  // Combine with another one
  State& operator+=(const State& o);

  // Set from JSON
  void set_from_json(const JSON::Value& json);

  // Get as JSON
  JSON::Value get_as_json() const;
};

typedef shared_ptr<State> StatePtr;

//==========================================================================
}} //namespaces
#endif // !__VG_DMX_H
