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
// Create an element with the given type - may be section:id or just id,
// which is looked up in default namespaces
GraphElement *Engine::create(const string& type, const string& id) const
{
  vector<string> bits = Text::split(type, ':');
  if (bits.size() > 1)
  {
    // Qualified - use the section given
    auto e = element_registry.create(bits[0], bits[1]);
    if (e)
      e->set_id(id);
    return e;
  }
  else
  {
    // Try default sections
    for (const auto& section: default_sections)
    {
      auto e = element_registry.create(section, type);
      if (e)
      {
        e->set_id(id);
        return e;
      }
    }

    return nullptr;
  }
}

//--------------------------------------------------------------------------
// Update element list
void Engine::update_elements()
{
  MT::RWReadLock lock(graph_mutex);
  tick_elements.clear();
  graph->collect_elements(tick_elements);
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
      const auto td = TickData{timestamp, sample_rate, nsamples};

      // Tick the graph
      MT::RWReadLock lock(graph_mutex);
      auto ticked = list<Element *>{};
      while (!tick_elements.empty())
      {
        for (auto it = tick_elements.begin(); it != tick_elements.end();)
        {
          if ((*it)->ready())
          {
            (*it)->tick(td);
            (*it)->reset();
            ticked.push_back(*it);
            it = tick_elements.erase(it);
          }
          else
          {
            ++it;
          }
        }
      }
      tick_elements = ticked;
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
// Accept visitors
void Engine::accept(ReadVisitor& visitor,
                    const Path& path, unsigned path_index) const
{
  MT::RWReadLock lock{graph_mutex};
  visitor.visit(*this, path, path_index);
  auto sv = visitor.get_root_graph_visitor();
  if (sv)
    graph->accept(*sv, path, path_index);
}

void Engine::accept(WriteVisitor& visitor,
                    const Path& path, unsigned path_index)
{
  MT::RWWriteLock lock{graph_mutex};
  visitor.visit(*this, path, path_index);
  auto sv = visitor.get_root_graph_visitor();
  if (sv)
    graph->accept(*sv, path, path_index);
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
