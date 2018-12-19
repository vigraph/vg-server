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
// MIDI interface
class Interface
{
 public:
  struct EventObserver
  {
    virtual void handle(ViGraph::MIDI::Event& event) = 0;
  };

  // Construct
  Interface() {}

  // Register an event handler for the given channel (or 0 if all) and
  // event type
  virtual void register_event_observer(int channel,
                                       ViGraph::MIDI::Event::Type type,
                                       EventObserver *observer) = 0;
};

//==========================================================================
}}} // namespaces
#endif // !__VIGRAPH_MIDI_SERVICES_H
