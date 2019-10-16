//==========================================================================
// ViGraph dataflow module: core/sources/selector/selector.cc
//
// Multigraph selector
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>

namespace {

//==========================================================================
// Selector source
class SelectorSource: public Dataflow::Source
{
public:
  bool retrigger{false};
  int index = -1;

private:
  unique_ptr<Dataflow::MultiGraph> multigraph;
  struct Start
  {
    Dataflow::timestamp_t t = 0.0;
    uint64_t n = 0;

    Start() {}
    Start(Dataflow::timestamp_t _t, uint64_t _n): t{_t}, n{_n} {}
    Start(const Start& s): t{s.t}, n{s.n} {}

    bool operator!() const
    {
      return !t && !n;
    }
  };
  map<int, Start> active_starts;  // Starts, zeroed when new

  // Source/Element virtuals
  void calculate_topology(Element::Topology& topo) override;
  void setup() override;
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

  void select_subgraph();
  void toggle_subgraph();
  void enable_subgraph();
  void disable_subgraph();
};

//--------------------------------------------------------------------------
// Topology calculation
void SelectorSource::calculate_topology(Element::Topology& topo)
{
  // Note we include all subs in the topology, whether or not activated,
  // to avoid having to recalculate the topology dynamically
  multigraph->calculate_topology(topo, this);
}

//--------------------------------------------------------------------------
// Setup after creation
void SelectorSource::setup()
{
  // Create empty multigraph if not already set from JSON or XML
  if (!multigraph)
    multigraph.reset(new Dataflow::MultiGraph(graph->get_engine(), graph));
}

//--------------------------------------------------------------------------
// Select a subgraph
void SelectorSource::select_subgraph()
{
  int old_index = -1;

  // Find single old one, if any, to allow increment
  if (active_starts.size() == 1)
    old_index = active_starts.begin()->first;

  if ((retrigger || index != old_index) && index >= 0)
  {
    Log::Detail log;
    log << "Selector source selected index " << index << endl;

    // Disable all current
    disable();

    // Clear all current and add this
    active_starts.clear();

    Dataflow::Graph *sub = multigraph->get_subgraph(index);
    if (sub)
    {
      active_starts[index] = {0.0, 0};
      // Enable it
      sub->enable();
    }
    else
    {
      Log::Error elog;
      elog << "Selector requested to select out-of-range item "
           << index << endl;
    }
  }
}

//--------------------------------------------------------------------------
// Enable a subgraph
void SelectorSource::enable_subgraph()
{
  Log::Detail log;
  log << "Selector source enabling index " << index << endl;

  Dataflow::Graph *sub = multigraph->get_subgraph(index);
  if (sub)
  {
    // Mark to start at next tick if not already there
    if (!active_starts[index])
      active_starts[index] = {0.0, 0};

    // Enable it
    sub->enable();
  }
  else
  {
    Log::Error elog;
    elog << "Selector requested to enable out-of-range item "
         << index << endl;
  }
}

//--------------------------------------------------------------------------
// Disable a subgraph
void SelectorSource::disable_subgraph()
{
  Log::Detail log;
  log << "Selector source disabling index " << index << endl;

  Dataflow::Graph *sub = multigraph->get_subgraph(index);
  if (sub)
  {
    // Remove from active
    active_starts.erase(index);

    // Disable
    sub->disable();
  }
  else
  {
    Log::Error elog;
    elog << "Selector requested to disable out-of-range item "
         << index << endl;
  }
}

//--------------------------------------------------------------------------
// Toggle a subgraph
void SelectorSource::toggle_subgraph()
{
  Log::Detail log;
  log << "Selector source toggling index " << index << endl;

  Dataflow::Graph *sub = multigraph->get_subgraph(index);
  if (sub)
  {
    auto it = active_starts.find(index);
    if (it == active_starts.end())
    {
      // Mark to start at next tick
      active_starts[index] = {0.0, 0};

      // Enable it
      sub->enable();
    }
    else
    {
      // Remove from active
      active_starts.erase(index);

      // Disable
      sub->disable();
    }
  }
  else
  {
    Log::Error elog;
    elog << "Selector requested to toggle out-of-range item "
         << index << endl;
  }
}

//--------------------------------------------------------------------------
// Enable all active subgraphs
void SelectorSource::enable()
{
  // Reset active
  active_starts.clear();
}

//--------------------------------------------------------------------------
// Disable all active subgraphs
void SelectorSource::disable()
{
  // Disable all active
  for(auto& it: active_starts)
  {
    Dataflow::Graph *sub = multigraph->get_subgraph(it.first);
    if (sub) sub->disable();
  }

  active_starts.clear();
}

//--------------------------------------------------------------------------
// Pre-tick
void SelectorSource::pre_tick(const TickData& td)
{
  // Pre-tick all active
  for(auto& it: active_starts)
  {
    Dataflow::Graph *sub = multigraph->get_subgraph(it.first);
    if (sub)
    {
      if (!it.second) it.second = {td.t, td.n};  // Reset datum time
      sub->pre_tick({td.t-it.second.t, td.n-it.second.n, td.interval,
                     td.sample_rate, td.global_t, td.global_n});
    }
  }
}

//--------------------------------------------------------------------------
// Generate a frame
void SelectorSource::tick(const TickData& td)
{
  // Tick all active
  for(auto& it: active_starts)
  {
    Dataflow::Graph *sub = multigraph->get_subgraph(it.first);
    if (sub) sub->tick({td.t-it.second.t, td.n-it.second.n, td.interval,
                        td.sample_rate, td.global_t, td.global_n});
  }
}

//--------------------------------------------------------------------------
// Post-tick
void SelectorSource::post_tick(const TickData& td)
{
  // Post-tick all active
  for(auto& it: active_starts)
  {
    Dataflow::Graph *sub = multigraph->get_subgraph(it.first);
    if (sub) sub->post_tick({td.t-it.second.t, td.n-it.second.n, td.interval,
                             td.sample_rate, td.global_t, td.global_n});
  }
}

//--------------------------------------------------------------------------
// Get JSON
JSON::Value SelectorSource::get_json(const string& path) const
{
  const auto& subgraphs = multigraph->get_subgraphs();

  // Whole selector?
  if (path.empty())
  {
    JSON::Value json = Source::get_json();
    JSON::Value& gsj = json.put("graphs", JSON::Value(JSON::Value::ARRAY));
    for(const auto& sgit: subgraphs)
    {
      JSON::Value& gj = gsj.add(JSON::Value(JSON::Value::OBJECT));
      gj.set("id", sgit.first);
      gj.set("elements", sgit.second->get_json(path));
    }
    return json;
  }
  else
  {
    // Select specific subgraph and pass it the rest of the path
    vector<string> bits = Text::split(path, '/', false, 3);
    if (bits[0] == "graph")
    {
      // Select specific subgraph and pass it the rest of the path
      const auto it = subgraphs.find(bits[1]);
      if (it == subgraphs.end())
        throw runtime_error("No such sub-graph "+bits[1]+" in selector");

      // Return bare value (or INVALID) up, undecorated
      return it->second->get_json(bits.size()>2 ? bits[2] : "");
    }
    else
    {
      // Our own property
      return Source::get_json(path);
    }
  }
}

//--------------------------------------------------------------------------
// Set from JSON
void SelectorSource::set_json(const string& path, const JSON::Value& value)
{
  // Whole selector?
  if (path.empty())
  {
    // Set our properties
    Element::set_json(path, value);

    // Create new multigraph
    multigraph.reset(new Dataflow::MultiGraph(graph->get_engine(), graph));

    // 'graphs' contains the array of sub-graphs
    const auto& graphs = value["graphs"];

    if (graphs.type != JSON::Value::ARRAY)
      throw runtime_error("Whole selector setting needs a 'graphs' JSON array");

    // Add each one as a new subgraph
    for(const auto& sg: graphs.a)
    {
      const auto& id = sg["id"].as_str();
      Graph *sub = new Dataflow::Graph(graph->get_engine(), graph);
      multigraph->add_subgraph(id, sub);
      sub->set_json(path, sg["elements"]);
    }
  }
  else
  {
    // If 'graph/xxx/yyy', pass down to specific subgraph
    vector<string> bits = Text::split(path, '/', false, 3);
    if (bits[0] == "graph")
    {
      // Select specific subgraph and pass it the rest of the path
      const auto& subgraphs = multigraph->get_subgraphs();
      const auto it = subgraphs.find(bits[1]);
      if (it == subgraphs.end())
        throw runtime_error("No such sub-graph "+bits[1]+" in selector");
      it->second->set_json(bits.size()>2 ? bits[2] : "", value);
    }
    else
    {
      // Set our own property
      Element::set_json(path, value);
    }
  }
}

//--------------------------------------------------------------------------
// Add to JSON
void SelectorSource::add_json(const string& path, const JSON::Value& value)
{
  // Whole selector?
  if (path.empty())
  {
    throw runtime_error("Can't add to entire selector contents");
  }
  else
  {
    // Select specific subgraph and pass it the rest of the path
    vector<string> bits = Text::split(path, '/', false, 3);
    if (bits[0] == "graph")
    {
      const auto& subgraphs = multigraph->get_subgraphs();
      const auto it = subgraphs.find(bits[1]);
      if (it == subgraphs.end())
      {
        // We need to create it
        Graph *sub = new Dataflow::Graph(graph->get_engine(), graph);
        multigraph->add_subgraph(bits[1], sub);
        sub->add_json(bits.size()>2 ? bits[2] : "", value);
      }
      else
      {
        it->second->add_json(bits.size()>2 ? bits[2] : "", value);
      }
    }
    else throw runtime_error("Can't add to a selector property");
  }
}

//--------------------------------------------------------------------------
// Delete item
void SelectorSource::delete_item(const string& path)
{
  // Whole selector?
  if (path.empty())
  {
    throw runtime_error("Can't delete entire selector contents");
  }
  else
  {
    // Select specific subgraph and pass it the rest of the path
    vector<string> bits = Text::split(path, '/', false, 3);
    if (bits[0] == "graph")
    {
      const auto& subgraphs = multigraph->get_subgraphs();
      const auto it = subgraphs.find(bits[1]);
      if (it == subgraphs.end())
        throw runtime_error("No such sub-graph "+bits[1]+" in selector");

      // If no sub-path, we are deleting the entire thing
      if (bits.size() > 2)
        it->second->delete_item(bits[2]);
      else
      {
        // Delete a subgraph
        multigraph->delete_subgraph(bits[1]);
      }
    }
    else throw runtime_error("Can't delete a selector property");
  }
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "selector",
  "Selector",
  "Selects one of several sub-graphs to tick",
  "core",
  {
    { "retrigger", { "Whether to retrigger same item", Value::Type::boolean,
                     &SelectorSource::retrigger, true } },
    { "value", { "Value to act upon", Value::Type::number,
                 &SelectorSource::index, true } },
    { "select", { "Select single item", Value::Type::trigger,
                  &SelectorSource::select_subgraph, true } },
    { "enable",   { "Item to enable", Value::Type::trigger,
                    &SelectorSource::enable_subgraph, true } },
    { "disable",  { "Item to disable", Value::Type::trigger,
                    &SelectorSource::disable_subgraph, true } },
    { "toggle",  { "Item to toggle", Value::Type::trigger,
                   &SelectorSource::toggle_subgraph, true } }
  },
  {}, // no inputs
  { "any" },
  true // container
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SelectorSource, module)
