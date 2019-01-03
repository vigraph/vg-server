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
  unique_ptr<Dataflow::MultiGraph> multigraph;
  map<int, Dataflow::timestamp_t> active_starts;  // Start time, or 0 when new

  // Source/Element virtuals
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override;
  void attach(Dataflow::Acceptor *_target) override;
  void set_property(const string& property, const SetParams& sp) override;
  void tick(Dataflow::timestamp_t t) override;

public:
  SelectorSource(const Module *module, const XML::Element& config):
    Element(module, config), Source(module, config) {}
};

//--------------------------------------------------------------------------
// Construct from XML:
//  <selector>
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

    if (index != old_index && index >= 0)
    {
      Log::Detail log;
      log << "Selector source selected index " << index << endl;

      // Clear all current and add this
      active_starts.clear();

      Dataflow::Graph *sub = multigraph->get_subgraph(index);
      if (sub)
        active_starts[index] = 0;
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
        active_starts[index] = 0;
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
// Generate a frame
void SelectorSource::tick(Dataflow::timestamp_t t)
{
  // Tick all active
  for(auto& it: active_starts)
  {
    Dataflow::Graph *sub = multigraph->get_subgraph(it.first);
    if (sub)
    {
      if (!it.second) it.second = t;  // Reset datum time
      sub->tick(t-it.second);
    }
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
    { "selected", { "Selected single item", Value::Type::number, true } },
    { "enable",   { "Item to enable", Value::Type::number, true } },
    { "disable",  { "Item to disable", Value::Type::number, true } }
  },
  {}, // no inputs
  { "any" },
  true // container
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SelectorSource, module)
