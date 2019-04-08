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

    // Configure it with graph in place so it can reach back to us for
    // other services
    srv->graph = graph.get();
    srv->configure(base_dir, e);
  }

  // Configure graph from graph config
  graph->configure(base_dir, graph_config);

  // Enable it
  graph->enable();
}

//------------------------------------------------------------------------
// Get state as a JSON value (see Graph::get_json())
JSON::Value Engine::get_json(const string& path) const
{
  MT::RWReadLock lock(graph_mutex);
  return graph->get_json(path);
}

//------------------------------------------------------------------------
// Set state from JSON
// path is a path/to/leaf/prop - can set any intermediate level too
void Engine::set_json(const string& path, const JSON::Value& value)
{
  MT::RWWriteLock lock(graph_mutex);
  graph->set_json(path, value);
}

//------------------------------------------------------------------------
// Set a new element from JSON
// path is a path/to/leaf
void Engine::add_json(const string& path, const JSON::Value& value)
{
  MT::RWWriteLock lock(graph_mutex);
  graph->add_json(path, value);
}

//------------------------------------------------------------------------
// Tick the engine
void Engine::tick(Time::Stamp t)
{
  if (!start_time) start_time = t;

  while (t >= start_time + tick_interval * tick_number)
  {
    timestamp_t timestamp = tick_interval.seconds() * tick_number;

    try
    {
      const auto td = TickData{timestamp, tick_number, tick_interval};
      // Tick all services
      for(const auto& it: services)
        it.second->tick(td);

      // Tick the graph
      MT::RWReadLock lock(graph_mutex);
      graph->pre_tick(td);
      graph->tick(td);
      graph->post_tick(td);
    }
    catch (runtime_error e)
    {
      Log::Error log;
      log << "Graph tick failed: " << e.what() << endl;
    }

    tick_number++;
  }
}

//------------------------------------------------------------------------
// Shut down the graph
void Engine::shutdown()
{
  // Shut down graph
  {
    MT::RWWriteLock lock(graph_mutex);
    graph->shutdown();
  }

  // Shut down services
  for(const auto& it: services)
    it.second->shutdown();
  services.clear();
}

}} // namespaces
