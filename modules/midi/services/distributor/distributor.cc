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
  void register_event_observer(int channel,
                               ViGraph::MIDI::Event::Type type,
                               EventObserver *observer) override;
  void deregister_event_observer(EventObserver *observer) override;
  void handle_event(const ViGraph::MIDI::Event& event) override;

  // Event observers
  struct Observer
  {
    int channel;
    ViGraph::MIDI::Event::Type type;
    EventObserver *observer;

    Observer(int _channel, ViGraph::MIDI::Event::Type _type,
             EventObserver *_observer):
      channel(_channel), type(_type), observer(_observer) {}
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
void MIDIDistributor::register_event_observer(int channel,
                                                ViGraph::MIDI::Event::Type type,
                                                EventObserver *observer)
{
  observers.push_back(Observer(channel, type, observer));
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
    if ((!o.channel || o.channel == event.channel) // channel 0 is wildcard
        && o.type == event.type)
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
