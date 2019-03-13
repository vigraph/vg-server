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
  unique_ptr<Dataflow::MultiGraph> multigraph;
  int n{1};

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
  CloneSource(const Module *module, const XML::Element& config):
    Source(module, config) {}
};

//--------------------------------------------------------------------------
// Configure from XML:
//  <clone n="10">
//    .. subgraph elements ..
//  </clone>
void CloneSource::configure(const File::Directory& base_dir,
                            const XML::Element& config)
{
  n = config.get_attr_int("n", 1);

  multigraph.reset(new Dataflow::MultiGraph(graph->get_engine()));
  multigraph->configure(base_dir, config);

  // Read our own children as sub-graphs, n times
  for(auto i=0; i<n; i++)
  {
    Graph *sub = multigraph->add_subgraph(base_dir, config);
    sub->set_variable("clone-number", Value{(double)(i+1)});
    sub->set_variable("clone-fraction", Value{(double)i/n});
  }
}

//--------------------------------------------------------------------------
// Attach an acceptor
// Overrides Generator::attach, attaches to all sub-graphs
void CloneSource::attach(Dataflow::Acceptor *acceptor)
{
  multigraph->attach_to_all(acceptor);
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
// Module definition
Dataflow::Module module
{
  "clone",
  "Clone",
  "Clones multiple copies of a sub-graph",
  "core",
  {
    { "n", { { "Number of copies to clone", "1" }, Value::Type::number } }
  },
  {}, // no inputs
  { "any" },
  true // container
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(CloneSource, module)
