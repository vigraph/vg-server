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
  if (config.get_attr_bool("thread"))
  {
    threaded = true;
    thread_serialiser.reset(new ThreadSerialiser{});
  }

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

  Graph *sub = new Graph(engine, parent);
  sub->configure(base_dir, graph_config);

  // Lock for write
  MT::RWWriteLock lock(mutex);
  subgraphs.push_back(shared_ptr<Graph>(sub));
  subgraphs_by_id[id] = sub;
  if (threaded)
    threads.emplace(piecewise_construct,
                    forward_as_tuple(sub), forward_as_tuple(subgraphs.back()));
  return sub;
}

//------------------------------------------------------------------------
// Calculate topology (see Element::calculate_topology)
void MultiGraph::calculate_topology(Element::Topology& topo, Element *owner)
{
  MT::RWReadLock lock(mutex);
  for(const auto it: subgraphs)
    it->calculate_topology(topo, owner);
}

//------------------------------------------------------------------------
// Attach an Acceptor to the end of all subgraphs (testing only)
void MultiGraph::attach_to_all(Acceptor *a)
{
  MT::RWReadLock lock(mutex);
  if (thread_serialiser)
  {
    thread_serialiser->attach(a);
    for(const auto it: subgraphs)
      it->attach(thread_serialiser.get());
  }
  else
  {
    for(const auto it: subgraphs)
      it->attach(a);
  }
}

//------------------------------------------------------------------------
// Attach an Acceptor Element to the end of all subgraphs
void MultiGraph::attach_to_all(Element *el)
{
  MT::RWReadLock lock(mutex);
  if (thread_serialiser)
  {
    thread_serialiser->attach(el);
    for(const auto it: subgraphs)
      it->attach(thread_serialiser.get());
  }
  else
  {
    for(const auto it: subgraphs)
      it->attach(el);
  }
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
  if (threaded)
  {
    for (auto& it: threads)
      it.second.pre_tick(td);
    for (auto& it: threads)
      it.second.wait();
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
  if (threaded)
  {
    for (auto& it: threads)
      it.second.tick(td);
    for (auto& it: threads)
      it.second.wait();
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
  if (threaded)
  {
    for (auto& it: threads)
      it.second.post_tick(td);
    for (auto& it: threads)
      it.second.wait();
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

//==========================================================================
// MultiGraph::Thread

//--------------------------------------------------------------------------
// Run a task
void MultiGraph::Thread::run(const TickData& _td, Task _task)
{
  td = _td;
  task = _task;
  start_c.signal();
}

//--------------------------------------------------------------------------
// Main thread loop
void MultiGraph::Thread::loop()
{
  auto stop = false;
  while (!stop)
  {
    start_c.wait();
    start_c.clear();
    switch (task)
    {
      case Task::pre_tick:
        graph->pre_tick(td);
        break;
      case Task::tick:
        graph->tick(td);
        break;
      case Task::post_tick:
        graph->post_tick(td);
        break;
      case Task::exit:
        stop = true;
        break;
    }
    done_c.signal();
  }
}

//--------------------------------------------------------------------------
// Wait for run to finish
void MultiGraph::Thread::wait()
{
  done_c.wait();
  done_c.clear();
}

//--------------------------------------------------------------------------
// Destructor
MultiGraph::Thread::~Thread()
{
  task = Task::exit;
  start_c.signal();
  wait();
  t.join();
}

}} // namespaces
