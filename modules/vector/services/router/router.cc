//==========================================================================
// ViGraph dataflow module:
//   vector/services/router/router.cc
//
// Spots collisions between frames sent by <collision-detect> elements, and
// triggers collision callbacks on all parties involved.
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"
#include "../../vector-services.h"

namespace {

//==========================================================================
// Collision Detector implementation
class RouterImpl: public Dataflow::Service, public Router
{
  map<string, list<Receiver *>> receivers;

  // Router implementation
  void register_receiver(const string& tag, Receiver *receiver) override;
  void deregister_receiver(Receiver *receiver) override;
  void send(const string& tag, FramePtr frame) override;

 public:
  // Construct
  RouterImpl(const Dataflow::Module *module, const XML::Element& config):
    Service(module, config)
  {}
};

//--------------------------------------------------------------------------
// Register for frame data on the given tag
void RouterImpl::register_receiver(const string& tag, Receiver *receiver)
{
  receivers[tag].push_back(receiver);
}

//--------------------------------------------------------------------------
// Deregister a receiver for all tags
void RouterImpl::deregister_receiver(Receiver *receiver)
{
  for(auto it: receivers)
    it.second.remove(receiver);
}

//--------------------------------------------------------------------------
// Send frame data on the given tag
void RouterImpl::send(const string& tag, FramePtr frame)
{
  const auto it = receivers.find(tag);
  if (it != receivers.end())
  {
    for (const auto it2: it->second)
      it2->receive(frame);
  }
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "router",
  "Router",
  "Router service, routes tagged data from senders to receivers",
  "vector",
  {} // no properties
};

} // anon

VIGRAPH_ENGINE_SERVICE_MODULE_INIT(RouterImpl, module)
