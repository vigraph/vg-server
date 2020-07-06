//==========================================================================
// ViGraph Ether Dream protocol library: status.cc
//
// Status parser
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-etherdream.h"
#include "ot-chan.h"

namespace ViGraph { namespace EtherDream {

// Read from raw data
bool Status::read(const vector<uint8_t>& data)
{
  if (data.size() < 20) return false;
  Channel::BlockReader br(data);
  try
  {
    protocol = br.read_byte();
    light_engine_state = (LightEngineState)br.read_byte();
    playback_state     = (PlaybackState)br.read_byte();
    source             = (Source)br.read_byte();
    light_engine_flags = br.read_le_16();
    playback_flags     = br.read_le_16();
    source_flags       = br.read_le_16();
    buffer_fullness    = br.read_le_16();
    point_rate         = br.read_le_32();
    point_count        = br.read_le_32();
  }
  catch (Channel::Error e)
  {
    return false;
  }

  return true;
}

}} // namespaces
