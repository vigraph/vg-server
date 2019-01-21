//==========================================================================
// ViGraph dataflow module: core/sources/graph/graph.cc
//
// Simple sub-graph
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"

namespace {

//==========================================================================
// Graph source
class GraphSource: public Dataflow::Source
{
  unique_ptr<Dataflow::Graph> subgraph;

  // Source/Element virtuals
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override;
  void attach(Dataflow::Acceptor *_target) override;
  void pre_tick(const TickData& td) override;
  void tick(const TickData& td) override;
  void post_tick(const TickData& td) override;
  void enable() override;
  void disable() override;

public:
  GraphSource(const Module *module, const XML::Element& config):
    Element(module, config), Source(module, config) {}
};

//--------------------------------------------------------------------------
// Configure from XML:
//  <graph>
//    .. subgraph elements ..
//  </graph>
void GraphSource::configure(const File::Directory& base_dir,
                            const XML::Element& config)
{
  subgraph.reset(new Dataflow::Graph(graph->get_engine()));
  subgraph->configure(base_dir, config);
}

//--------------------------------------------------------------------------
// Attach an acceptor
// Overrides Generator::attach, attaches to sub-graph
void GraphSource::attach(Dataflow::Acceptor *acceptor)
{
  subgraph->attach(acceptor);
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
