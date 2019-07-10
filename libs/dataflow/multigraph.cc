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
// Set send-up function
void MultiGraph::set_send_up_function(Graph::SendUpFunction f)
{
  // Set send-up function to pass on sub-graph's output, either serialising
  // for threaded, or pass straight through
  if (threaded)
    send_up_function = [this, f](DataPtr data)
      { MT::Lock lock(send_up_serialisation_mutex); f(data); };
  else
    send_up_function = f;

  // Set on any existing graphs
  MT::RWReadLock lock(mutex);
  for(const auto it: subgraphs)
    it->set_send_up_function(send_up_function);
}

//------------------------------------------------------------------------
// Add a graph
// Returns sub-Graph (owned by us)
Graph *MultiGraph::add_subgraph()
{
  const auto id = string{"graph-"} + Text::itos(++id_serial);

  Graph *sub = new Graph{engine};
  sub->set_send_up_function(send_up_function);

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
// Add a pre-constructed sub-graph
void MultiGraph::add_subgraph(const string& id, Graph *sub)
{
  // Lock for write
  MT::RWWriteLock lock(mutex);
  subgraphs.push_back(shared_ptr<Graph>(sub));
  subgraphs_by_id[id] = sub;
  if (threaded)
    threads.emplace(piecewise_construct,
                    forward_as_tuple(sub), forward_as_tuple(subgraphs.back()));
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
