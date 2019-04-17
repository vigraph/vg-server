//==========================================================================
// ViGraph DMX modules: dmx-services.h
//
// Definitions of shared services for DMX modules
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_DMX_SERVICES_H
#define __VIGRAPH_DMX_SERVICES_H

namespace ViGraph { namespace Module { namespace DMX {

typedef byte dmx_value_t;

//--------------------------------------------------------------------------
// Direction
enum class Direction
{
  in,
  out,
};

//==========================================================================
// DMX distributor
class Distributor
{
 public:
  struct EventObserver
  {
    virtual void handle(unsigned universe, unsigned channel,
                        dmx_value_t value) = 0;
  };

  // Construct
  Distributor() {}

  // Register an event handler
  virtual void register_event_observer(Direction direction,
                                       unsigned min_universe,
                                       unsigned max_universe,
                                       unsigned min_channel,
                                       unsigned max_channel,
                                       EventObserver *observer) = 0;

  // Deregister observer for all events
  virtual void deregister_event_observer(EventObserver *observer) = 0;

  // Handle a DMX event
  virtual void handle_event(Direction direction, unsigned universe,
                            unsigned channel, dmx_value_t value) = 0;
  virtual void handle_event(Direction direction, unsigned universe,
                            const vector<dmx_value_t>& values) = 0;
};

//==========================================================================
}}} // namespaces
#endif // !__VIGRAPH_DMX_SERVICES_H
