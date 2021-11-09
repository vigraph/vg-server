//==========================================================================
// ViGraph Nexus: vg-nexus-queue.h
//
// Queue library for Nexus server
//
// Copyright (c) 2021 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VG_NEXUS_QUEUE_H
#define __VG_NEXUS_QUEUE_H

#include "ot-json.h"
#include "ot-mt.h"
#include "ot-time.h"

namespace ViGraph { namespace Nexus {

// Make our lives easier without polluting anyone else
using namespace std;
using namespace ObTools;
using namespace ViGraph;

// Queue of clients
class Queue
{
  // Configuration
  Time::Duration active_time;  // Time they get at the head of the queue

  // Dynamic state
  MT::Mutex mutex;
  list<string> waiting;
  string current;
  Time::Stamp current_start_time;

public:
  // Constructor
  Queue(const Time::Duration& _active_time = {}): active_time(_active_time) {}

  // Get the active time
  Time::Duration get_active_time() { return active_time; }

  // Set the active time
  void set_active_time(const Time::Duration& t) { active_time = t; }

  // Add an ID to the end of the queue
  void add(const string& id, const Time::Stamp& now);

  // Remove an ID from anywhere in the queue
  void remove(const string& id);

  // Check for new head at the given time
  // Returns new head ID, or "" if no change
  string check_time_up(const Time::Stamp& now);

  // Get the current queue, in order, not including the current one
  // Note it's a copy, for thread safety
  list<string> get_waiting() { MT::Lock lock(mutex); return waiting; }

  // Get the currently active ID, if any
  string get_active() { MT::Lock lock(mutex); return current; }

  // Get the time remaining for the active ID (seconds)
  int get_time_remaining(const Time::Stamp& now);
};

//==========================================================================
}} //namespaces
#endif // !__VG_NEXUS_QUEUE_H
