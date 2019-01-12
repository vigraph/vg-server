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
  void tick(Dataflow::timestamp_t t) override;
  void enable() override;
  void disable() override;

public:
  CloneSource(const Module *module, const XML::Element& config):
    Element(module, config), Source(module, config) {}
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

  // Read our own children as sub-graphs, n times
  for(auto i=0; i<n; i++)
    multigraph->add_subgraph(base_dir, config);
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
// Generate a frame
void CloneSource::tick(Dataflow::timestamp_t t)
{
  multigraph->tick_all(t);
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
  { "VectorFrame" },
  true // container
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(CloneSource, module)
