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
  bool retrigger{false};
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
  void set_property(const string& property, const SetParams& sp) override;
  void pre_tick(const TickData& td) override;
  void tick(const TickData& td) override;
  void post_tick(const TickData& td) override;
  void enable() override;
  void disable() override;

public:
  SelectorSource(const Module *module, const XML::Element& config):
    Element(module, config), Source(module, config) {}
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
// Set a control property
void SelectorSource::set_property(const string& property, const SetParams& sp)
{
  if (property == "selected")
  {
    int index = -1;

    // Find single old one, if any, to allow increment
    if (active_starts.size() == 1)
      index = active_starts.begin()->first;

    // Update it
    int old_index = index;
    update_prop_int(index, sp);

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
  else if (property == "enable")
  {
    int index{0};
    update_prop_int(index, sp);

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
  else if (property == "disable")
  {
    int index{0};
    update_prop_int(index, sp);

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
  else if (property == "toggle")
  {
    int index{0};
    update_prop_int(index, sp);

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
      elog << "Selector requested to disable out-of-range item "
           << index << endl;
    }
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
// Module definition
Dataflow::Module module
{
  "selector",
  "Selector",
  "Selects one of several sub-graphs to tick",
  "core",
  {
    { "retrigger", { "Whether to retrigger same item", Value::Type::boolean } },
    { "selected", { "Selected single item", Value::Type::number, true } },
    { "enable",   { "Item to enable", Value::Type::number, true } },
    { "disable",  { "Item to disable", Value::Type::number, true } },
    { "toggle",  { "Item to toggle", Value::Type::number, true } }
  },
  {}, // no inputs
  { "any" },
  true // container
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SelectorSource, module)
