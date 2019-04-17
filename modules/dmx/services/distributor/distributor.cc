//==========================================================================
// ViGraph dataflow module: dmx/services/distributor/distributor.cc
//
// Generic DMX event distributor
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include "../../dmx-services.h"

using namespace ViGraph::Module::DMX;

namespace {

//==========================================================================
// DMX distributor
class DMXDistributor: public Dataflow::Service, public Distributor
{
  // DMX interface implementation
  void register_event_observer(Direction direction,
                               unsigned min_universe, unsigned max_universe,
                               unsigned min_channel, unsigned max_channel,
                               EventObserver *observer) override;
  void deregister_event_observer(EventObserver *observer) override;
  void handle_event(Direction direction, unsigned universe, unsigned channel,
                    dmx_value_t value) override;
  void handle_event(Direction direction, unsigned universe,
                    const vector<dmx_value_t>&) override;

  // Event observers
  struct Observer
  {
    Direction direction;
    unsigned min_universe = 0;
    unsigned max_universe = 0;
    unsigned min_channel = 0;
    unsigned max_channel = 0;
    EventObserver *observer = nullptr;

    // Constructor
    Observer(Direction direction,
             unsigned _min_universe, unsigned _max_universe,
             unsigned _min_channel, unsigned _max_channel,
             EventObserver *_observer):
      direction(direction),
      min_universe(_min_universe), max_universe(_max_universe),
      min_channel(_min_channel), max_channel(_max_channel),
      observer(_observer) {}
  };

  list<Observer> observers;

public:
  using Service::Service;
};

//--------------------------------------------------------------------------
// Register an event handler
void DMXDistributor::register_event_observer(Direction direction,
                                unsigned min_universe, unsigned max_universe,
                                unsigned min_channel, unsigned max_channel,
                                EventObserver *observer)
{
  observers.emplace_back(direction, min_universe, max_universe,
                         min_channel, max_channel, observer);
}

//--------------------------------------------------------------------------
// Deregister an event observer for all events
void DMXDistributor::deregister_event_observer(EventObserver *observer)
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
// Handle a DMX value
void DMXDistributor::handle_event(Direction direction, unsigned universe,
                                  unsigned channel, dmx_value_t value)
{
  // Send event to all interested observers
  for (const auto& o: observers)
  {
    if (direction == o.direction &&
        (universe >= o.min_universe && universe <= o.max_universe) &&
        (channel >= o.min_channel && channel <= o.max_channel))
    {
      o.observer->handle(universe, channel, value);
    }
  }
}

//--------------------------------------------------------------------------
// Handle a set of DMX values
void DMXDistributor::handle_event(Direction direction, unsigned universe,
                                  const vector<dmx_value_t>& values)
{
  // Send event to all interested observers
  for (const auto& o: observers)
  {
    if (direction == o.direction &&
        (universe >= o.min_universe && universe <= o.max_universe))
    {
      auto channel = 1u;
      for (auto v: values)
      {
        if (channel >= o.min_channel && channel <= o.max_channel)
        {
          o.observer->handle(universe, channel, v);
        }
        ++channel;
      }
    }
  }
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "distributor",
  "DMX Distributor",
  "DMX event distributor",
  "dmx",
  {}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(DMXDistributor, module)
