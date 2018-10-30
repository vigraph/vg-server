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
  int selected_index{-1};
  bool selection_changed{false};
  Dataflow::timestamp_t last_change{0};

  // Source/Element virtuals
  void configure(const XML::Element& config) override;
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
void SelectorSource::configure(const XML::Element& config)
{
  multigraph.reset(new Dataflow::MultiGraph(graph->get_engine()));
  multigraph->configure(config);
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
  if (property == "index")
  {
    int old_index = selected_index;
    update_prop_int(selected_index, sp);
    if (selected_index != old_index)
    {
      Log::Detail log;
      log << "Selector source selected index " << selected_index << endl;
      selection_changed = true;
    }
  }
}

//--------------------------------------------------------------------------
// Generate a frame
void SelectorSource::tick(Dataflow::timestamp_t t)
{
  // Notice changes and reset clock
  if (selection_changed)
  {
    last_change = t;
    selection_changed = false;
  }

  if (selected_index >= 0)
  {
    Dataflow::Graph *sub = multigraph->get_subgraph(selected_index);
    if (sub) sub->tick(t-last_change);
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
    { "index", { "Selected index", Value::Type::number, true } }
  },
  {}, // no inputs
  { "VectorFrame" },
  true // container
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SelectorSource, module)
