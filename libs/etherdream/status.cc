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

// Dump to channel
void Status::dump(ostream& out)
{
  out << "Status:\n";
  out << "  Light engine: ";
  switch (light_engine_state)
  {
    case LightEngineState::ready:
      out << "Ready";
      break;

    case LightEngineState::warmup:
      out << "Warmup";
      break;

    case LightEngineState::cooldown:
      out << "Cooldown";
      break;

    case LightEngineState::e_stop:
      out << "E-Stop";
      break;
  }

  out << "\n  Playback: ";
  switch (playback_state)
  {
    case PlaybackState::idle:
      out << "Idle";
      break;

    case PlaybackState::prepared:
      out << "Prepared";
      break;

    case PlaybackState::playing:
      out << "Playing";
      break;
  }

  out << "\n  Source: ";
  switch (source)
  {
    case Source::network:
      out << "Network";
      break;

    case Source::sd_card:
      out << "SD card";
      break;

    case Source::internal:
      out << "Internal";
      break;
  }

  out << "\n  Light engine flags:";
  if (light_engine_flags & LightEngineFlags::e_stop_network)
    out << " e-stop-network";
  if (light_engine_flags & LightEngineFlags::e_stop_external)
    out << " e-stop-external";
  if (light_engine_flags & LightEngineFlags::e_stop_active)
    out << " e-stop-active";
  if (light_engine_flags & LightEngineFlags::e_stop_over_temperature)
    out << " e-stop-temperature";
  if (light_engine_flags & LightEngineFlags::over_temperature_active)
    out << " over-temperature-active";
  if (light_engine_flags & LightEngineFlags::e_stop_no_link)
    out << " e-stop-no-link";

  out << "\n  Playback flags:";
  if (playback_flags & PlaybackFlags::shutter_open)
    out << " shutter-open";
  if (playback_flags & PlaybackFlags::underflow)
    out << " underflow";
  if (playback_flags & PlaybackFlags::e_stop)
    out << " e-stop";

  out << "\n  Buffer fullness: " << buffer_fullness << endl;
  out << "  Point rate: " << point_rate << endl;
  out << "  Point count: " << point_count << endl;
}

}} // namespaces
