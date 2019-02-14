//==========================================================================
// ViGraph dataflow module: core/services/pool/pool.cc
//
// Sends control events to a single provider in a named pool
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include "../../core-services.h"

namespace {

using namespace ViGraph::Module::Core;

//==========================================================================
// Pool Distributor
class PoolDistributorImpl: public Dataflow::Service, public PoolDistributor
{
  struct Pool
  {
    struct Worker
    {
      Control *worker = nullptr;
      double on = -1.0;
      Time::Stamp on_at;

      Worker(Control *_worker): worker{_worker} {}

      bool operator<(const Worker& b)
      {
        return on_at < b.on_at;
      }
    };
    list<Worker> workers;
  };
  map<string, Pool> pools;

  // Pool implementation
  void register_worker(const string& pool, Control *worker) override;
  void deregister_worker(Control *worker) override;
  void send(const string& pool, const string& prop,
            const Dataflow::Control::SetParams& sp) override;

 public:
  // Construct
  PoolDistributorImpl(const Dataflow::Module *module,
                      const XML::Element& config):
    Service(module, config)
  {}
};

//--------------------------------------------------------------------------
// Register for frame data on the given tag
void PoolDistributorImpl::register_worker(const string& pool, Control *worker)
{
  pools[pool].workers.emplace_back(worker);
}

//--------------------------------------------------------------------------
// Deregister a worker for all tags
void PoolDistributorImpl::deregister_worker(Control *worker)
{
  for (auto& p: pools)
  {
    for (auto it = p.second.workers.begin(); it != p.second.workers.end();)
    {
      if (it->worker == worker)
      {
        if (it->on >= 0)
          it->worker->set_property("off", Value{it->on});
        p.second.workers.erase(it++);
      }
      else
      {
        ++it;
      }
    }
  }
}

//--------------------------------------------------------------------------
// Send frame data on the given tag
void PoolDistributorImpl::send(const string& pool, const string& prop,
                           const SetParams& sp)
{
  const auto pit = pools.find(pool);
  if (pit != pools.end() && !pit->second.workers.empty())
  {
    if (prop == "on")
    {
      for (auto& w: pit->second.workers)
      {
        if (w.on < 0)
        {
          w.worker->set_property("on", sp);
          w.on = sp.v.d;
          w.on_at = Time::Stamp::now();
          return;
        }
      }
      pit->second.workers.sort();
      auto& w = pit->second.workers.front();
      if (w.on >= 0)
        w.worker->set_property("off", Value{w.on});
      w.worker->set_property("on", sp);
      w.on = sp.v.d;
      w.on_at = Time::Stamp::now();
    }
    else if (prop == "off")
    {
      for (auto& w: pit->second.workers)
      {
        if (w.on == sp.v.d)
        {
          w.worker->set_property("off", sp);
          w.on = -1.0;
          return;
        }
      }
    }
  }
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "pool-distributor",
  "Pool Distributor",
  "Pool Distributor service, finds next available provider for an on/off",
  "core",
  {} // no properties
};

} // anon

VIGRAPH_ENGINE_SERVICE_MODULE_INIT(PoolDistributorImpl, module)
