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
  using Service::Service;

  void set_min_spares(int min_spares)
  { func_pool.set_min_spares(min_spares); }
  int get_min_spares() const
  { return func_pool.get_min_spares(); }
  void set_max_threads(int max_threads)
  { func_pool.set_max_threads(max_threads); }
  int get_max_threads() const
  { return func_pool.get_max_threads(); }
};

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "thread-pool",
  "Thread Pool",
  "Thread Pool service, provides parallel processing",
  "core",
  {
    { "min-spares", { "Minimum spare threads", Value::Type::number,
                      { &ThreadPoolImpl::get_min_spares,
                        &ThreadPoolImpl::set_min_spares }, true } },
    { "max-threads", { "Maximum worker threads", Value::Type::number,
                      { &ThreadPoolImpl::get_max_threads,
                        &ThreadPoolImpl::set_max_threads }, true } },
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ThreadPoolImpl, module)
