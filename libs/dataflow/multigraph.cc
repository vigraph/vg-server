//==========================================================================
// ViGraph dataflow machines: multigraph.cc
//
// Multiple graph container implementation
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-dataflow.h"

namespace ViGraph { namespace Dataflow {

//------------------------------------------------------------------------
// Construct with XML
// Throws a runtime_error if configuration fails
void MultiGraph::configure(const File::Directory& base_dir,
                           const XML::Element& config)
{
  // Set the optional thread pool
  if (!config.get_attr("thread-pool").empty())
    thread_pool = engine.get_service<ThreadPool>("thread-pool");

  // Load all <graph> elements in config
  for(const auto& p: config.get_children("graph"))
    add_subgraph(base_dir, *p);
}

//------------------------------------------------------------------------
// Add a graph from the given XML
// Throws a runtime_error if configuration fails
// Returns sub-Graph (owned by us)
Graph *MultiGraph::add_subgraph(const File::Directory& base_dir,
                                const XML::Element& graph_config)
{
  string id = graph_config["id"];
  if (id.empty()) id = "graph-"+Text::itos(++id_serial);

  Graph *sub = new Graph(engine);
  sub->configure(base_dir, graph_config);

  // Lock for write
  MT::RWWriteLock lock(mutex);
  subgraphs.push_back(shared_ptr<Graph>(sub));
  subgraphs_by_id[id] = sub;
  return sub;
}

//------------------------------------------------------------------------
// Attach an Acceptor to the end of all subgraphs (testing only)
void MultiGraph::attach_to_all(Acceptor *a)
{
  MT::RWReadLock lock(mutex);
  for(const auto it: subgraphs)
    it->attach(a);
}

//------------------------------------------------------------------------
// Attach an Acceptor Element to the end of all subgraphs
void MultiGraph::attach_to_all(Element *el)
{
  MT::RWReadLock lock(mutex);
  for(const auto it: subgraphs)
    it->attach(el);
}

//------------------------------------------------------------------------
// Enable all subgraphs
void MultiGraph::enable_all()
{
  MT::RWReadLock lock(mutex);
  for(const auto it: subgraphs)
    it->enable();
}

//------------------------------------------------------------------------
// Disable all subgraphs
void MultiGraph::disable_all()
{
  MT::RWReadLock lock(mutex);
  for(const auto it: subgraphs)
    it->disable();
}

//------------------------------------------------------------------------
// Pre-tick all subgraphs
void MultiGraph::pre_tick_all(const TickData& td)
{
  MT::RWReadLock lock(mutex);
  if (thread_pool)
  {
    auto functions = vector<function<void()>>{};
    for (const auto it: subgraphs)
      functions.emplace_back([it, &td]() { it->pre_tick(td); });
    thread_pool->run_and_wait(functions);
  }
  else
  {
    for(const auto it: subgraphs)
      it->pre_tick(td);
  }
}

//------------------------------------------------------------------------
// Tick all subgraphs
void MultiGraph::tick_all(const TickData& td)
{
  MT::RWReadLock lock(mutex);
  if (thread_pool)
  {
    auto functions = vector<function<void()>>{};
    for (const auto it: subgraphs)
      functions.emplace_back([it, &td]() { it->tick(td); });
    thread_pool->run_and_wait(functions);
  }
  else
  {
    for(const auto it: subgraphs)
      it->tick(td);
  }
}

//------------------------------------------------------------------------
// Post-tick all subgraphs
void MultiGraph::post_tick_all(const TickData& td)
{
  MT::RWReadLock lock(mutex);
  if (thread_pool)
  {
    auto functions = vector<function<void()>>{};
    for (const auto it: subgraphs)
      functions.emplace_back([it, &td]() { it->post_tick(td); });
    thread_pool->run_and_wait(functions);
  }
  else
  {
    for(const auto it: subgraphs)
      it->post_tick(td);
  }
}

//------------------------------------------------------------------------
// Get a particular graph by ID
Graph *MultiGraph::get_subgraph(const string& id)
{
  MT::RWReadLock lock(mutex);
  if (subgraphs_by_id.find(id) != subgraphs_by_id.end())
    return subgraphs_by_id[id];
  else
    return nullptr;
}

//------------------------------------------------------------------------
// Get a particular graph by index
Graph *MultiGraph::get_subgraph(size_t index)
{
  MT::RWReadLock lock(mutex);
  if (index < subgraphs.size())
    return subgraphs[index].get();
  else
    return nullptr;
}

//------------------------------------------------------------------------
// Shutdown all subgraphs
void MultiGraph::shutdown()
{
  MT::RWWriteLock lock(mutex);
  for(const auto it: subgraphs)
    it->shutdown();
}


}} // namespaces
