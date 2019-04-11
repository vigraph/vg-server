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
      unsigned last_index = 0;
      bool cleared = true;
      Time::Stamp last_used;

      Worker(Control *_worker): worker{_worker} {}

      bool operator<(const Worker& b)
      {
        return last_used < b.last_used;
      }

      void send(unsigned index, const string& property,
                const Dataflow::Value& v)
      {
        worker->set_property(property, v);
        last_index = index;
        cleared = (property == "clear");
        last_used = Time::Stamp::now();
      }
    };
    list<Worker> workers;
  };
  map<string, Pool> pools;

  // Pool implementation
  void register_worker(const string& pool, Control *worker) override;
  void deregister_worker(Control *worker) override;
  void send(const string& pool, unsigned index,
            const string& property,
            const Dataflow::Value& v) override;

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
        if (!it->cleared)
          it->worker->trigger("clear");
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
void PoolDistributorImpl::send(const string& pool,
                               unsigned index,
                               const string& property,
                               const Value& v)
{
  const auto pit = pools.find(pool);
  if (pit != pools.end() && !pit->second.workers.empty())
  {
    // Look for matching index
    for (auto& w: pit->second.workers)
    {
      if (w.last_index == index)
      {
        w.send(index, property, v);
        return;
      }
    }

    // Look for worker that has been cleared the longest
    pit->second.workers.sort();
    for (auto& w: pit->second.workers)
    {
      if (w.cleared)
      {
        w.send(index, property, v);
        return;
      }
    }

    // Pick the one that hasn't been used for the longest time
    // (front due to sorting)
    auto& w = pit->second.workers.front();
    // Clear previous recipient
    w.worker->trigger("clear");
    w.send(index, property, v);
  }
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "pool-distributor",
  "Pool Distributor",
  "Pool Distributor service distributes messages based on pool name and index",
  "core",
  {} // no properties
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(PoolDistributorImpl, module)
