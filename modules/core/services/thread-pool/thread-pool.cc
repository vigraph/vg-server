//==========================================================================
// ViGraph dataflow module: core/services/thread-pool/thread-pool.cc
//
// Provides a pool of threads for running general functions on
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include "../../core-services.h"

namespace {

using namespace ViGraph::Module::Core;

//==========================================================================
// Thread Pool
class ThreadPoolImpl: public Dataflow::Service, public ThreadPool
{
  MT::FunctionPool func_pool;

  // Pool implementation
  bool run(function<void()> f)
  {
    return func_pool.run(f);
  }

  void run_and_wait(vector<function<void()>>& vf)
  {
    func_pool.run_and_wait(vf);
  }

 public:
  // Construct
  ThreadPoolImpl(const Dataflow::Module *module, const XML::Element& config):
    Service(module, config),
    func_pool(config.get_attr_int("min-spares", 1),
              config.get_attr_int("max-workers", 10))
  {

  }
};

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "thread-pool",
  "Thread Pool",
  "Thread Pool service, provides parallel processing",
  "core",
  {} // no properties
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ThreadPoolImpl, module)
