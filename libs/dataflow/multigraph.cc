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
  // Load all <graph> elements in config
  for(const auto& p: config.get_children("graph"))
    add_subgraph(base_dir, *p);
}

//------------------------------------------------------------------------
// Add a graph from the given XML
// Throws a runtime_error if configuration fails
void MultiGraph::add_subgraph(const File::Directory& base_dir,
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
// Tick all subgraphs
void MultiGraph::tick_all(timestamp_t t)
{
  MT::RWReadLock lock(mutex);
  for(const auto it: subgraphs)
    it->tick(t);
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
