//==========================================================================
// ViGraph UI modules: ui-services.h
//
// Definitions of shared services for UI modules
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_UI_SERVICES_H
#define __VIGRAPH_UI_SERVICES_H

namespace ViGraph { namespace Module { namespace UI {

//==========================================================================
// Key distributor
class KeyDistributor
{
 public:
  struct KeyObserver
  {
    virtual void handle_key(int code) = 0;
  };

  // Construct
  KeyDistributor() {}

  // Register a key handler for the given key (negative when released)
  virtual void register_key_observer(int code,
                                     KeyObserver *observer) = 0;

  // Deregister an observer from all keys
  virtual void deregister_key_observer(KeyObserver *observer) = 0;

  // Handle a key
  virtual void handle_key(int code) = 0;
};

//==========================================================================
}}} // namespaces
#endif // !__VIGRAPH_UI_SERVICES_H
