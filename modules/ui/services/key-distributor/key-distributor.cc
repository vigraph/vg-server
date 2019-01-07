//==========================================================================
// ViGraph dataflow module: ui/services/key-distributor/key-distributor.cc
//
// Distributes key codes to <key> controls
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include "../../ui-services.h"

using namespace ViGraph::Module::UI;

namespace {

//--------------------------------------------------------------------------
class KeyDistributorImpl: public Dataflow::Service, public KeyDistributor
{
  // Key observers
  struct Observer
  {
    int code;
    KeyObserver *observer;

    Observer(int _code, KeyObserver *_observer):
      code(_code), observer(_observer) {}
  };

  list<Observer> observers;

  // KeyDistributor virtuals
  void register_key_observer(int code, KeyObserver *observer) override;
  void deregister_key_observer(KeyObserver *observer) override;
  void handle_key(int code) override;

public:
  // Construct
  KeyDistributorImpl(const Dataflow::Module *module,
                     const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <key-distributor/>
KeyDistributorImpl::KeyDistributorImpl(const Dataflow::Module *module,
                                       const XML::Element& config):
  Service(module, config)
{
}

//--------------------------------------------------------------------------
// Register a key observer
void KeyDistributorImpl::register_key_observer(int code,
                                               KeyObserver *observer)
{
  observers.push_back(Observer(code, observer));
}

//--------------------------------------------------------------------------
// Deregister an observer from all keys
void KeyDistributorImpl::deregister_key_observer(KeyObserver *observer)
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
// Handle a key
void KeyDistributorImpl::handle_key(int code)
{
  for (const auto& obs: observers)
  {
    if (obs.code == code) obs.observer->handle_key(code);
  }
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "key-distributor",
  "Key Distributor",
  "Accepts key presses and distributes them to <key> controls",
  "ui",
  {}
};

} // anon

VIGRAPH_ENGINE_SERVICE_MODULE_INIT(KeyDistributorImpl, module)
