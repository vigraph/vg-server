//==========================================================================
// ViGraph dataflow module: core/sources/graph/graph.cc
//
// Simple sub-graph
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <atomic>

namespace {

//==========================================================================
// Graph source
class GraphSource: public Dataflow::Source
{
  unique_ptr<Dataflow::Graph> subgraph;

  // Source/Element virtuals
  void calculate_topology(Element::Topology& topo) override;
  void pre_tick(const TickData& td) override;
  void tick(const TickData& td) override;
  void post_tick(const TickData& td) override;
  void enable() override;
  void disable() override;
  void shutdown() override;
  JSON::Value get_json(const string& path) const override;
  void set_json(const string& path, const JSON::Value& value) override;
  void add_json(const string& path, const JSON::Value& value) override;
  void delete_item(const string& path) override;

public:
  double sample_rate = 0;

  using Source::Source;
};

//--------------------------------------------------------------------------
// Topology calculation
void GraphSource::calculate_topology(Element::Topology& topo)
{
  subgraph->calculate_topology(topo, this);  // Use us as proxy
}

//--------------------------------------------------------------------------
// Enable subgraph
void GraphSource::enable()
{
  subgraph->enable();
}

//--------------------------------------------------------------------------
// Disable subgraph
void GraphSource::disable()
{
  subgraph->disable();
}

//--------------------------------------------------------------------------
// Pre-tick
void GraphSource::pre_tick(const TickData& td)
{
  subgraph->pre_tick(td);
}

//--------------------------------------------------------------------------
// Generate a frame
void GraphSource::tick(const TickData& td)
{
  subgraph->tick(td);
}

//--------------------------------------------------------------------------
// Post-tick
void GraphSource::post_tick(const TickData& td)
{
  subgraph->post_tick(td);
}

//--------------------------------------------------------------------------
// Shut down the subgraph
void GraphSource::shutdown()
{
  subgraph->shutdown();
}

//--------------------------------------------------------------------------
// Get JSON
JSON::Value GraphSource::get_json(const string& path) const
{
  if (path.empty())
  {
    // Whole graph structure at this level
    JSON::Value json = Source::get_json();
    json.set("elements", subgraph->get_json());
    return json;
  }
  else
  {
    // Just the undecorated object the graph returns
    return subgraph->get_json(path);
  }
}

//--------------------------------------------------------------------------
// Set from JSON
void GraphSource::set_json(const string& path, const JSON::Value& value)
{
  // Whole graph?
  if (path.empty())
  {
    // Set our properties (if we ever have any)
    Element::set_json(path, value);

    // Create the subgraph
    subgraph.reset(new Dataflow::Graph(*engine, graph));

    // 'elements' contains the array of sub-elements - we can just pass
    // direct to the subgraph
    const auto& elements = value["elements"];
    subgraph->set_json(path, elements);
  }
  else
  {
    subgraph->set_json(path, value);
  }
}

//--------------------------------------------------------------------------
// Add to JSON
void GraphSource::add_json(const string& path, const JSON::Value& value)
{
  if (path.empty())
  {
    const auto& elements = value["elements"];
    subgraph->set_json(path, elements);
  }
  else
  {
    subgraph->add_json(path, value);
  }
}

//--------------------------------------------------------------------------
// Delete an item
void GraphSource::delete_item(const string& path)
{
  if (path.empty())
  {
    throw runtime_error("Can't delete a whole graph - needs a sub-element ID");
  }
  else
  {
    subgraph->delete_item(path);
  }
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "graph",
  "Graph",
  "Creates a single sub-graph",
  "core",
  {}, // no properties
  {}, // no inputs
  { "any" },
  true // container
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(GraphSource, module)
