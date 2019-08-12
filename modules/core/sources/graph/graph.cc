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
    Graph *parent;

    File::Path source_file;
    Time::Duration check_interval;

    void run() override
    {
      try
      {
        subgraph.reset(new Dataflow::Graph(engine, parent));
        subgraph->configure(source_file, check_interval);
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
                 Engine& _engine, Graph *_parent,
                 const File::Path& _source_file,
                 const Time::Duration& _check_interval):
      subgraph{updated_subgraph}, status{update_status},
      engine{_engine}, parent{_parent},
      source_file{_source_file}, check_interval{_check_interval}
    {
      start();
    }
  };
  unique_ptr<UpdateThread> update_thread;

  // Source/Element virtuals
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override;
  void setup() override;
  void calculate_topology(Element::Topology& topo) override;
  void pre_tick(const TickData& td) override;
  void tick(const TickData& td) override;
  void post_tick(const TickData& td) override;
  void enable() override;
  void disable() override;
  void shutdown() override;
  JSON::Value get_json(const string& path) const override;
  void set_json(const string& path, const JSON::Value& value) override;
  void add_json(const string& path, const JSON::Value& value) override;
  void delete_item(const string& path) override;

public:
  double sample_rate = 0;

  GraphSource(const Module *module, const XML::Element& config):
    Source(module, config)
  {}
};

//--------------------------------------------------------------------------
// Configure from XML:
//  <graph>
//    .. subgraph elements ..
//  </graph>
void GraphSource::configure(const File::Directory& base_dir,
                            const XML::Element& config)
{
  Source::configure(base_dir, config);
  subgraph.reset(new Dataflow::Graph(graph->get_engine(), graph));
  subgraph->configure(base_dir, config);

  // Pass upgoing data straight on as if we were the source
  subgraph->set_send_up_function([this](DataPtr data) { send(data); });
}

//--------------------------------------------------------------------------
// Setup after creation
void GraphSource::setup()
{
  // Create empty subgraph if not already set from JSON or XML
  if (!subgraph)
    subgraph.reset(new Dataflow::Graph(graph->get_engine(), graph));
}

//--------------------------------------------------------------------------
// Topology calculation
void GraphSource::calculate_topology(Element::Topology& topo)
{
  subgraph->calculate_topology(topo, this);  // Use us as proxy
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
        if (subgraph->requires_update(source_file, check_interval))
        {
          update_status = UpdateStatus::updating;
          update_thread.reset(new UpdateThread{updated_subgraph, update_status,
                                               graph->get_engine(), graph,
                                               source_file, check_interval});
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
// Get JSON
JSON::Value GraphSource::get_json(const string& path) const
{
  if (path.empty())
  {
    // Whole graph structure at this level
    JSON::Value json = Source::get_json();
    json.set("elements", subgraph->get_json());
    return json;
  }
  else if (path == "elements")
  {
    // Just the sub-elements
    return subgraph->get_json();
  }
  else
  {
    // Just the undecorated object the graph returns
    return subgraph->get_json(path);
  }
}

//--------------------------------------------------------------------------
// Set from JSON
void GraphSource::set_json(const string& path, const JSON::Value& value)
{
  // Whole graph?
  if (path.empty())
  {
    // Set our properties (if we ever have any)
    Element::set_json(path, value);

    // 'elements' contains the array of sub-elements - we can just pass
    // direct to the subgraph
    const auto& elements = value["elements"];
    subgraph->set_json(path, elements);
  }
  else
  {
    subgraph->set_json(path, value);
  }
}

//--------------------------------------------------------------------------
// Add to JSON
void GraphSource::add_json(const string& path, const JSON::Value& value)
{
  if (path.empty())
  {
    const auto& elements = value["elements"];
    subgraph->set_json(path, elements);
  }
  else
  {
    subgraph->add_json(path, value);
  }
}

//--------------------------------------------------------------------------
// Delete an item
void GraphSource::delete_item(const string& path)
{
  if (path.empty())
  {
    throw runtime_error("Can't delete a whole graph - needs a sub-element ID");
  }
  else
  {
    subgraph->delete_item(path);
  }
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
