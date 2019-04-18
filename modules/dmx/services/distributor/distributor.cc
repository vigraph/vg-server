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
  map<unsigned, vector<dmx_value_t>> current_values;

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
  auto new_values = vector<bool>(values.size());
  // For incoming data we only care about changes, so figure out what's new
  if (direction == Direction::in)
  {
    auto& current_uni = current_values[universe];
    auto len = min(current_uni.size(), values.size());
    for (auto i = 0u; i < len; ++i)
    {
      if (current_uni[i] != values[i])
      {
        new_values[i] = true;
        current_uni[i] = values[i];
      }
    }
    if (current_uni.size() < values.size())
    {
      new_values.insert(new_values.end(), values.size() - current_uni.size(),
                        true);
      copy(values.begin() + current_uni.size(), values.end(),
           back_inserter(current_uni));
    }
  }
  // Send event to all interested observers
  for (const auto& o: observers)
  {
    if (direction == o.direction &&
        (universe >= o.min_universe && universe <= o.max_universe))
    {
      auto channel = 1u;
      auto is_new = new_values.begin();
      for (auto v: values)
      {
        if (*is_new)
        {
          if (channel >= o.min_channel && channel <= o.max_channel)
          {
            o.observer->handle(universe, channel, v);
          }
        }
        ++channel;
        ++is_new;
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
