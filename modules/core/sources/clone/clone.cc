//==========================================================================
// ViGraph dataflow module: core/sources/clone/clone.cc
//
// Graph cloner
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"

namespace {

//==========================================================================
// Clone source
class CloneSource: public Dataflow::Source
{
public:
  int n{1};

private:
  unique_ptr<Dataflow::MultiGraph> multigraph;

  // Source/Element virtuals
  void calculate_topology(Element::Topology& topo) override;
  void pre_tick(const TickData& td) override;
  void tick(const TickData& td) override;
  void post_tick(const TickData& td) override;
  void enable() override;
  void disable() override;
  JSON::Value get_json(const string& path) const override;
  void set_json(const string& path, const JSON::Value& value) override;
  void add_json(const string& path, const JSON::Value& value) override;
  void delete_item(const string& path) override;

public:
  using Source::Source;
};

//--------------------------------------------------------------------------
// Topology calculation
void CloneSource::calculate_topology(Element::Topology& topo)
{
  multigraph->calculate_topology(topo, this);  // Use us as proxy
}

//--------------------------------------------------------------------------
// Enable all subgraphs
void CloneSource::enable()
{
  multigraph->enable_all();
}

//--------------------------------------------------------------------------
// Disable all subgraphs
void CloneSource::disable()
{
  multigraph->disable_all();
}

//--------------------------------------------------------------------------
// Pre-tick
void CloneSource::pre_tick(const TickData& td)
{
  multigraph->pre_tick_all(td);
}

//--------------------------------------------------------------------------
// Generate a frame
void CloneSource::tick(const TickData& td)
{
  multigraph->tick_all(td);
}

//--------------------------------------------------------------------------
// Post-tick
void CloneSource::post_tick(const TickData& td)
{
  multigraph->post_tick_all(td);
}

//--------------------------------------------------------------------------
// Get JSON
JSON::Value CloneSource::get_json(const string& path) const
{
  // Just use the first (assuming there are any)
  Graph *subgraph = multigraph->get_subgraph(0);

  if (path.empty())
  {
    // Whole structure
    JSON::Value json = Source::get_json();
    if (subgraph) json.set("graph", subgraph->get_json(path));
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
void CloneSource::set_json(const string& path, const JSON::Value& value)
{
  // Whole thing?
  if (path.empty())
  {
    // Set our properties (including 'n')
    Element::set_json(path, value);

    // Create new multigraph
    multigraph.reset(new Dataflow::MultiGraph(*engine));

    // 'graph' contains the array of sub-elements - we can just pass
    // direct to the subgraphs
    const auto& elements = value["graph"];

    // Create children as sub-graphs, n times
    for(auto i=0; i<n; i++)
    {
      Graph *sub = multigraph->add_subgraph();
      sub->set_variable("clone-number", Value{(double)(i+1)});
      sub->set_variable("clone-fraction", Value{(double)i/n});
      sub->set_json(path, elements);
    }
  }
  else
  {
    // If 'graph/xxx', pass down to cloned subgraphs
    vector<string> bits = Text::split(path, '/', false, 2);
    if (bits[0] == "graph")
    {
      const auto& subgraphs = multigraph->get_subgraphs();
      for(const auto& sub: subgraphs)
        sub.second->set_json(bits.size()>1 ? bits[1] : "", value);
    }
    else
    {
      // Set our own property ('n' only)
      Element::set_json(path, value);
    }
  }
}

//--------------------------------------------------------------------------
// Add from JSON
void CloneSource::add_json(const string& path, const JSON::Value& value)
{
  // Whole thing?
  if (path.empty())
  {
    throw runtime_error("Can't add to entire clone contents");
  }
  else
  {
    // If 'graph/xxx', pass down to cloned subgraphs
    vector<string> bits = Text::split(path, '/', false, 2);
    if (bits[0] == "graph")
    {
      const auto& subgraphs = multigraph->get_subgraphs();
      for(const auto& sub: subgraphs)
        sub.second->add_json(bits.size()>1 ? bits[1] : "", value);
    }
    else throw runtime_error("Can't add directly to a <clone>");
  }
}

//--------------------------------------------------------------------------
// Delete item
void CloneSource::delete_item(const string& path)
{
  // Whole thing?
  if (path.empty())
  {
    throw runtime_error("Can't delete entire clone contents");
  }
  else
  {
    // If 'graph/xxx', pass down to cloned subgraphs
    vector<string> bits = Text::split(path, '/', false, 2);
    if (bits[0] == "graph")
    {
      const auto& subgraphs = multigraph->get_subgraphs();
      for(const auto& sub: subgraphs)
        sub.second->delete_item(bits.size()>1 ? bits[1] : "");
    }
    else throw runtime_error("Can't delete directly from a <clone>");
  }
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "clone",
  "Clone",
  "Clones multiple copies of a sub-graph",
  "core",
  {
    { "n", { "Number of copies to clone", Value::Type::number,
             &CloneSource::n, false } }
  },
  {}, // no inputs
  { "any" },
  true // container
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(CloneSource, module)
