//==========================================================================
// ViGraph modules: module-services.h
//
// Definitions of shared services across all modules
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_MODULE_SERVICES_H
#define __VIGRAPH_MODULE_SERVICES_H

#include "vg-dataflow.h"

namespace ViGraph { namespace Module {

//==========================================================================
// Router interface
class Router
{
 public:
  struct Receiver
  {
    virtual void receive(DataPtr data) = 0;
  };

  // Construct
  Router() {}

  // Register for frame data on the given tag
  virtual void register_receiver(const string& tag, Receiver *receiver) = 0;

  // Deregister a receiver for all tags
  virtual void deregister_receiver(Receiver *receiver) = 0;

  // Send frame data on the given tag
  virtual void send(const string& tag, DataPtr data) = 0;

  // Virtual destructor
  virtual ~Router() {}
};

//==========================================================================
}} // namespaces
#endif // !__VIGRAPH_MODULE_SERVICES_H
