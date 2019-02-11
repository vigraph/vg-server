//==========================================================================
// ViGraph MIDI modules: midi-services.h
//
// Definitions of shared services for MIDI modules
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_MIDI_SERVICES_H
#define __VIGRAPH_MIDI_SERVICES_H

#include "vg-midi.h"

namespace ViGraph { namespace Module { namespace MIDI {

//==========================================================================
// MIDI distributor
class Distributor
{
 public:
  struct EventObserver
  {
    virtual void handle(const ViGraph::MIDI::Event& event) = 0;
  };

  // Construct
  Distributor() {}

  // Register an event handler for the given channel (or 0 if all) and
  // event type
  virtual void register_event_observer(
                                  ViGraph::MIDI::Event::Direction direction,
                                  unsigned min_channel,
                                  unsigned max_channel,
                                  ViGraph::MIDI::Event::Type type,
                                  EventObserver *observer) = 0;

  // Deregister observer for all events
  virtual void deregister_event_observer(EventObserver *observer) = 0;

  // Handle a MIDI event
  virtual void handle_event(const ViGraph::MIDI::Event& event) = 0;
};

//==========================================================================
}}} // namespaces
#endif // !__VIGRAPH_MIDI_SERVICES_H
