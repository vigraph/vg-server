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
  unique_ptr<Dataflow::Graph> updated_subgraph;
  enum class UpdateStatus {
    none,
    updating,
    updated,
    failed
  };
  atomic<UpdateStatus> update_status{UpdateStatus::none};

  //------------------------------------------------------------------------
  // Graph Update thread
  class UpdateThread: public MT::Thread
  {
  private:
    unique_ptr<Dataflow::Graph>& subgraph;
    atomic<UpdateStatus>& status;

    Engine& engine;

    File::Path source_file;
    Time::Duration check_interval;
    Acceptor *acceptor;

    void run() override
    {
      try
      {
        subgraph.reset(new Dataflow::Graph(engine));
        subgraph->configure(source_file, check_interval, acceptor);
        status = UpdateStatus::updated;
      }
      catch (...)
      {
        status = UpdateStatus::failed;
      }
    }

  public:
    UpdateThread(unique_ptr<Dataflow::Graph>& updated_subgraph,
                 atomic<UpdateStatus>& update_status,
                 Engine& _engine,
                 const File::Path& _source_file,
                 const Time::Duration& _check_interval,
                 Acceptor *_acceptor):
      subgraph{updated_subgraph}, status{update_status},
      engine{_engine},
      source_file{_source_file}, check_interval{_check_interval},
      acceptor{_acceptor}
    {
      start();
    }
  };
  unique_ptr<UpdateThread> update_thread;

  // Source/Element virtuals
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override;
  void attach(Dataflow::Acceptor *_target) override;
  void pre_tick(const TickData& td) override;
  void tick(const TickData& td) override;
  void post_tick(const TickData& td) override;
  void enable() override;
  void disable() override;
  void shutdown() override;

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
  switch (update_status.load())
  {
    case UpdateStatus::none:
      {
        auto source_file = File::Path{};
        auto check_interval = Time::Duration{};
        auto acceptor = static_cast<Acceptor *>(nullptr);
        if (subgraph->requires_update(source_file, check_interval, acceptor))
        {
          update_status = UpdateStatus::updating;
          update_thread.reset(new UpdateThread{updated_subgraph, update_status,
                                               graph->get_engine(),
                                               source_file, check_interval,
                                               acceptor});
        }
      }
      break;

    case UpdateStatus::updating:
      break;

    case UpdateStatus::updated:
      update_thread.reset();
      subgraph.swap(updated_subgraph);
      updated_subgraph->shutdown();
      updated_subgraph.reset();
      subgraph->enable();
      update_status = UpdateStatus::none;
      break;

    case UpdateStatus::failed:
      update_thread.reset();
      if (updated_subgraph)
        updated_subgraph->shutdown();
      updated_subgraph.reset();
      update_status = UpdateStatus::none;
      break;
  }
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
  update_thread.reset();
  if (updated_subgraph)
    updated_subgraph->shutdown();
  subgraph->shutdown();
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
