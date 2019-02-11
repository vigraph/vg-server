//==========================================================================
// ViGraph dataflow module: midi/services/distributor/distributor.cc
//
// Generic MIDI event distributor
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include "../../midi-services.h"
#include "vg-midi.h"

using namespace ViGraph::Module::MIDI;

namespace {

//==========================================================================
// MIDI distributor
class MIDIDistributor: public Dataflow::Service, public Distributor
{
  // MIDI interface implementation
  void register_event_observer(ViGraph::MIDI::Event::Direction direction,
                               unsigned min_channel, unsigned max_channel,
                               ViGraph::MIDI::Event::Type type,
                               EventObserver *observer) override;
  void deregister_event_observer(EventObserver *observer) override;
  void handle_event(const ViGraph::MIDI::Event& event) override;

  // Event observers
  struct Observer
  {
    ViGraph::MIDI::Event::Direction direction;
    unsigned min_channel = 0;
    unsigned max_channel = 0;
    ViGraph::MIDI::Event::Type type;
    EventObserver *observer;

    // Constructor
    Observer(ViGraph::MIDI::Event::Direction direction,
             unsigned _min_channel,  unsigned _max_channel,
             ViGraph::MIDI::Event::Type _type, EventObserver *_observer):
      direction(direction),
      min_channel(_min_channel), max_channel(_max_channel),
      type(_type), observer(_observer) {}
  };

  list<Observer> observers;

public:
  // Construct
  MIDIDistributor(const Dataflow::Module *module,
                  const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <midi-distributor/>
MIDIDistributor::MIDIDistributor(const Dataflow::Module *module,
                                     const XML::Element& config):
  Service(module, config)
{
}

//--------------------------------------------------------------------------
// Register an event handler - channel=0 means all (Omni)
void MIDIDistributor::register_event_observer(
                                  ViGraph::MIDI::Event::Direction direction,
                                  unsigned min_channel, unsigned max_channel,
                                  ViGraph::MIDI::Event::Type type,
                                  EventObserver *observer)
{
  observers.emplace_back(direction, min_channel, max_channel, type, observer);
}

//--------------------------------------------------------------------------
// Deregister an event observer for all events
void MIDIDistributor::deregister_event_observer(EventObserver *observer)
{
  for(auto p=observers.begin(); p!=observers.end();)
  {
    Observer& o = *p;
    if (o.observer == observer)
      p = observers.erase(p);
    else
      ++p;
  }
}

//--------------------------------------------------------------------------
// Handle a MIDI event
void MIDIDistributor::handle_event(const ViGraph::MIDI::Event& event)
{
  // Send event to all interested observers
  for(const auto& o: observers)
  {
    if (o.direction == event.direction
        &&
        (!o.min_channel || // channel 0 is wildcard
         (event.channel >= o.min_channel && event.channel <= o.max_channel))
        &&
        (o.type == ViGraph::MIDI::Event::Type::none ||
         o.type == event.type))
      o.observer->handle(event);
  }
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "midi-distributor",
  "MIDI Distributor",
  "MIDI event distributor",
  "midi",
  {}
};

} // anon

VIGRAPH_ENGINE_SERVICE_MODULE_INIT(MIDIDistributor, module)
