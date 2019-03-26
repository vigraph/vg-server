//==========================================================================
// ViGraph cores: core-services.h
//
// Definitions of shared services across core modules
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_CORE_SERVICES_H
#define __VIGRAPH_CORE_SERVICES_H

#include "vg-dataflow.h"

namespace ViGraph { namespace Module { namespace Core {

//==========================================================================
// Pool Distributor interface
class PoolDistributor
{
public:
  // Register a worker on/off control events from a pool
  virtual void register_worker(const string& pool, Control *worker) = 0;

  // Deregister an worker
  virtual void deregister_worker(Control *worker) = 0;

  // Send frame data on the given tag
  virtual void send(const string& pool,
                    unsigned index, const string& property,
                    const Dataflow::Control::SetParams& sp) = 0;

  // Virtual destructor
  virtual ~PoolDistributor() {}
};

//==========================================================================
}}} // namespaces
#endif // !__VIGRAPH_CORE_SERVICES_H
