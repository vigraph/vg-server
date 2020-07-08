//==========================================================================
// ViGraph Ether Dream protocol library: interface.cc
//
// Main interface implementation
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-etherdream.h"
#include "ot-log.h"

namespace ViGraph { namespace EtherDream {

// Get response with status as last_status
// Returns true if OK
bool Interface::get_response()
{
  for(;;)
  {
    if (receive_buffer.size() >= 22)  // 2 + status response
    {
      auto response = receive_buffer.front();

      // Remove response and command, which we ignore
      receive_buffer.erase(receive_buffer.begin(), receive_buffer.begin()+2);

      if (!last_status.read(receive_buffer))
        return false;

      if (response == 'a') return true;

      Log::Error log;
      log << "Ether Dream error response: " << response << " - ";
      switch (response)
      {
        case 'F': log << "buffer full"; break;
        case 'I': log << "invalid"; break;
        case '!': log << "emergency stop"; break;
        default: log << "UNKNOWN!";
      }

      log << endl;
      last_status.dump(log);
      return false;
    }

    // Get some more
    if (!channel.receive(receive_buffer))
      return false;
  }
}

// Start the interface
// Returns whether initial 'response' received and ping accepted
bool Interface::start()
{
  Log::Streams log;

  if (!get_response())
  {
    log.error << "Device startup response not received\n";
    return false;
  }

  commands.ping();

  if (!get_response())
  {
    log.error << "Device ping response not received\n";
    return false;
  }

  log.detail << "Ether Dream: connected OK\n";
  return true;
}

// Ensure the interface is ready to receive data
// Returns whether it is ready
bool Interface::get_ready()
{
  Log::Streams log;

  // Check for e-stop, reset it
  if (last_status.light_engine_state == Status::LightEngineState::e_stop)
  {
    log.summary << "Ether Dream: Trying to clear e-stop state\n";
    commands.clear_emergency_stop();
    if (!get_response())
    {
      log.error << "Ether Dream clearing e-stop failed\n";
      return false;
    }

    if (last_status.light_engine_state == Status::LightEngineState::e_stop)
    {
      log.error << "Ether Dream stuck in e-stop\n";
      return false;
    }

    log.detail << "Ether Dream: e-stop cleared OK\n";
  }

  // Check for idle, go prepared
  if (last_status.playback_state == Status::PlaybackState::idle)
  {
    log.detail << "Ether Dream: preparing\n";
    commands.prepare();
    if (!get_response())
    {
      log.error << "Ether Dream prepare failed\n";
      return false;
    }

    if (last_status.playback_state == Status::PlaybackState::idle)
    {
      log.error << "Ether Dream stuck in idle\n";
      return false;
    }

    log.detail << "Ether Dream: prepared OK\n";
  }

  return true;
}

// Send point data to the interface
// Returns whether data sent successfully
bool Interface::send(const vector<Point>& points)
{
  commands.send(points);
  if (!get_response())
  {
    Log::Streams log;
    log.error << "Ether Dream sending points failed\n";
    return false;
  }

  // Check for running, and if not, start it
  if (last_status.playback_state != Status::PlaybackState::playing)
  {
    Log::Streams log;
    log.detail << "Ether Dream: Start playing\n";
    commands.begin_playback(point_rate);
    if (!get_response())
    {
      log.error << "Ether Dream start playback failed\n";
      return false;
    }

    if (last_status.playback_state != Status::PlaybackState::playing)
    {
      log.error << "Ether Dream won't start playing\n";
      return false;
    }
  }

  return true;
}

}} // namespaces
