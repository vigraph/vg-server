//==========================================================================
// ViGraph dataflow machines: engine.cc
//
// Global dataflow engine implementation
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-dataflow.h"
#include "ot-log.h"

namespace ViGraph { namespace Dataflow {

//------------------------------------------------------------------------
// Configure with <graph> and <services> XML
// Throws a runtime_error if configuration fails
void Engine::configure(const File::Directory& base_dir,
                       const XML::Element& graph_config,
                       const XML::Element& services_config)
{
  // Create services from services config
  for (const auto& p: services_config.children)
  {
    const auto& e = *p;
    if (e.name.empty()) continue;

    const auto srv = service_registry.create(e.name, e);
    if (!srv) throw(runtime_error("No such dataflow service " + e.name));

    services[e.name].reset(srv);
  }

  // Configure graph from graph config
  graph->configure(base_dir, graph_config);

  // Enable it
  graph->enable();
}

//------------------------------------------------------------------------
// Tick the engine
void Engine::tick(Time::Stamp t)
{
  if (t - last_graph_tick_time >= tick_interval)
  {
    try
    {
      // Tick all services
      for(const auto& it: services)
        it.second->tick(timestamp);

      // Tick the graph
      MT::Lock lock(graph_mutex);
      graph->pre_tick(timestamp);
      graph->tick(timestamp);
      graph->post_tick(timestamp);
    }
    catch (runtime_error e)
    {
      Log::Error log;
      log << "Graph tick failed: " << e.what() << endl;
    }

    // Maintain constant timebase unless it's badly out (more than 1 interval)
    last_graph_tick_time += tick_interval;
    if (t - last_graph_tick_time >= tick_interval)
      last_graph_tick_time = t;
    timestamp += tick_interval.seconds();
  }
}

//------------------------------------------------------------------------
// Shut down the graph
void Engine::shutdown()
{
  // Shut down graph
  {
    MT::Lock lock(graph_mutex);
    graph->shutdown();
  }

  // Shut down services
  for(const auto& it: services)
    it.second->shutdown();
  services.clear();
}

}} // namespaces
