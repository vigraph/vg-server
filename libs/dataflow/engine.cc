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

//--------------------------------------------------------------------------
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

//--------------------------------------------------------------------------
// Delete an item (from REST)
// path is a path/to/leaf
void Engine::delete_item(const string& path)
{
  MT::RWWriteLock lock(graph_mutex);
  graph->delete_item(path);
}

//--------------------------------------------------------------------------
// Tick the engine
void Engine::tick(Time::Stamp t)
{
  if (!start_time) start_time = t;

  while (t >= start_time + tick_interval * tick_number)
  {
    const auto timestamp = tick_interval.seconds() * tick_number;

    try
    {
      const auto sample_rate = get_sample_rate();
      const auto last_tick_total = static_cast<size_t>(
        floor(tick_interval.seconds() * tick_number * sample_rate));
      const auto tick_total = static_cast<size_t>(
        floor(tick_interval.seconds() * (tick_number + 1) * sample_rate));
      const auto nsamples = tick_total - last_tick_total;

      // Tick the graph
      MT::RWReadLock lock(graph_mutex);
      graph->tick({timestamp, sample_rate, nsamples});
    }
    catch (const runtime_error& e)
    {
      Log::Error log;
      log << "Graph tick failed: " << e.what() << endl;
    }

    tick_number++;
  }
}

//--------------------------------------------------------------------------
// Accept a visitor
void Engine::accept(Visitor& visitor, bool write)
{
  auto lock = shared_ptr<void>{};
  if (write)
    lock.reset(new MT::RWWriteLock(graph_mutex));
  else
    lock.reset(new MT::RWReadLock(graph_mutex));
  visitor.visit(*this);
  auto sv = visitor.getSubGraphVisitor();
  if (sv)
    graph->accept(*sv);
}

//--------------------------------------------------------------------------
// Shut down the graph
void Engine::shutdown()
{
  // Shut down graph
  MT::RWWriteLock lock(graph_mutex);
  graph->shutdown();
}

}} // namespaces
