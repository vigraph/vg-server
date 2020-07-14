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

static const int min_fullness_for_start = 1500;

// Log stats at intervals
void Interface::log_stats()
{
  if (last_status.buffer_fullness > stats.max_fullness)
    stats.max_fullness = last_status.buffer_fullness;
  if (last_status.buffer_fullness < stats.min_fullness)
    stats.min_fullness = last_status.buffer_fullness;

  Time::Stamp now = Time::Stamp::now();
  if (now - stats.last_log_time >= stats.log_interval)
  {
    Log::Detail log;
    log << "Ether Dream stats: fullness " << last_status.buffer_fullness
        << " min " << stats.min_fullness
        << " max " << stats.max_fullness
        << endl;

    stats.last_log_time = now;
    stats.max_fullness = 0;
    stats.min_fullness = UINT16_MAX;
  }
}

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

      // Log error conditions
      if (last_status.playback_flags & Status::PlaybackFlags::underflow)
      {
        Log::Error log;
        log << "Ether Dream: Underflow\n";
      }

      if (last_status.playback_flags & Status::PlaybackFlags::e_stop)
      {
        Log::Error log;
        log << "Ether Dream: E-Stop!\n";
      }

      if (response == 'a')
      {
        log_stats();
        return true;
      }

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
// duration of this frame in seconds
// Returns whether data sent successfully
bool Interface::send(const vector<Point>& points,
                     double duration)
{
  // Calculate point rate for this period
  uint32_t point_rate = (uint32_t)(points.size() / duration + 0.5);
  commands.queue_rate_change(point_rate);
  if (!get_response())
  {
    Log::Streams log;
    log.error << "Ether Dream queue rate change failed\n";
    return false;
  }

  // Check for prepared after status from queue_rate_change
  get_ready();

  // Send points with rate change
  commands.send(points, true);
  if (!get_response())
  {
    Log::Streams log;
    log.error << "Ether Dream sending points failed\n";
    return false;
  }

  // Check for running, and if not, start it, after allowing buffer to
  // build up
  if (last_status.playback_state != Status::PlaybackState::playing
      && last_status.buffer_fullness >= min_fullness_for_start)
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

// Get estimate of buffer availability
size_t Interface::get_buffer_points_available()
{
  // !!! For now, get up-to-date fullness with a ping
  commands.ping();
  get_response();

  // !!! Quantify buffer size with Jacob
  // !!! Take into account points used since last send
  // !!! If async, take into account points sent but not acked
  auto available = 3000 - last_status.buffer_fullness;

  return available;
}

}} // namespaces
