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
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override;
  void attach(Dataflow::Acceptor *_target) override;
  void pre_tick(const TickData& td) override;
  void tick(const TickData& td) override;
  void post_tick(const TickData& td) override;
  void enable() override;
  void disable() override;
  JSON::Value get_json(const string& path) const override;

public:
  using Source::Source;

  void select_subgraph();
  void toggle_subgraph();
  void enable_subgraph();
  void disable_subgraph();
};

//--------------------------------------------------------------------------
// Construct from XML:
//  <selector retrigger="false">
//    <graph id="sub1">
//      ..
//    </graph>
//    <graph id="sub2"/>
//      ..
//    </graph>
//  </selector>
void SelectorSource::configure(const File::Directory& base_dir,
                               const XML::Element& config)
{
  Source::configure(base_dir, config);

  retrigger = config.get_attr_bool("retrigger");
  multigraph.reset(new Dataflow::MultiGraph(graph->get_engine()));
  multigraph->configure(base_dir, config);
}

//--------------------------------------------------------------------------
// Attach an acceptor
// Overrides Generator::attach, attaches to all sub-graphs
void SelectorSource::attach(Dataflow::Acceptor *acceptor)
{
  multigraph->attach_to_all(acceptor);
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
                     td.global_t, td.global_n});
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
                        td.global_t, td.global_n});
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
                             td.global_t, td.global_n});
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
    JSON::Value json = Element::get_json();
    JSON::Value& gsj = json.set("graphs", JSON::Value(JSON::Value::ARRAY));
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
    vector<string> bits = Text::split(path, '/', false, 2);
    const auto it = subgraphs.find(bits[0]);
    if (it == subgraphs.end())
      throw runtime_error("No such sub-graph "+bits[0]+" in selector");

    // Return bare value (or INVALID) up, undecorated
    return it->second->get_json(bits.size()>1 ? bits[1] : "");
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
