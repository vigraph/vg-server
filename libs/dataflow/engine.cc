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
// Create an element with the given name - may be section:id or just id,
// which is looked up in default namespaces
Element *Engine::create(const string& name)
{
  vector<string> bits = Text::split(name, ':');
  if (bits.size() > 1)
  {
    // Qualified - use the section given
    return element_registry.create(bits[0], bits[1]);
  }
  else
  {
    // Try default sections
    for(const auto& section: default_sections)
    {
      Element *e = element_registry.create(section, name);
      if (e) return e;
    }

    return nullptr;
  }
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
// Delete an item (from REST)
// path is a path/to/leaf
void Engine::delete_item(const string& path)
{
  MT::RWWriteLock lock(graph_mutex);
  graph->delete_item(path);
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
      const auto td = TickData{timestamp, tick_number, tick_interval,
                               get_sample_rate()};

      // Tick the graph
      MT::RWReadLock lock(graph_mutex);
      graph->pre_tick(td);
      graph->tick(td);
      graph->post_tick(td);
    }
    catch (const runtime_error& e)
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
  MT::RWWriteLock lock(graph_mutex);
  graph->shutdown();
}

}} // namespaces
