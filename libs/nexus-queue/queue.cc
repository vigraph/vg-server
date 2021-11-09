//==========================================================================
// ViGraph Nexus Queue: queue.cc
//
// Implementation of the queue
//
// Copyright (c) 2021 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-nexus-queue.h"
#include <math.h>

namespace ViGraph { namespace Nexus {

// Add an ID to the end of the queue
void Queue::add(const string& id, const Time::Stamp& now)
{
  MT::Lock lock(mutex);
  if (current.empty())
  {
    current = id;
    current_start_time = now;
  }
  else
    waiting.push_back(id);
}

// Remove an ID from anywhere in the queue
void Queue::remove(const string& id)
{
  MT::Lock lock(mutex);
  if (current == id)
    current = "";   // Marker for check to pop it
  else
    waiting.remove(id);
}

// Check for new head at the given time
// Returns current head ID
string Queue::check_time_up(const Time::Stamp& now)
{
  MT::Lock lock(mutex);

  // Time up, or removed?
  if (current.empty() || now >= current_start_time + active_time)
  {
    if (!waiting.empty())
    {
      current = waiting.front();
      current_start_time = now;
      waiting.pop_front();
    }
    else current = "";
  }

  return current;
}

// Get the time remaining for the active ID (seconds)
int Queue::get_time_remaining(const Time::Stamp& now)
{
  MT::Lock lock(mutex);
  if (current.empty()) return 0;
  return (int)ceil((active_time - (now - current_start_time)).seconds());
}

}} // namespaces
