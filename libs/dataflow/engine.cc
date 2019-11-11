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
  vector<string> bits = Text::split(type, namespace_separator);
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
  tick_elements.clear();
  graph->collect_elements(tick_elements);
}

//--------------------------------------------------------------------------
// Tick the engine
void Engine::tick(const Time::Duration& t)
{
  if (!start_time) start_time = t;

  const auto latest_tick_number = static_cast<unsigned long>((t - start_time)
                                                             / tick_interval);
  if (latest_tick_number > tick_number + 2)
  {
    Log::Error elog;
    const auto skip = latest_tick_number - tick_number - 2;
    elog << "Skipping " << skip << " ticks due to lag" << endl;
    tick_number += skip;
  }

  while (t >= start_time + tick_interval * tick_number)
  {
    const auto tick_start = tick_interval.seconds() * tick_number;
    const auto tick_end = tick_interval.seconds() * (tick_number + 1);

    try
    {
      const auto td = TickData{tick_start, tick_end};

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
// Shut down the graph
void Engine::shutdown()
{
  // Shut down graph
  MT::RWWriteLock lock(graph_mutex);
  graph->shutdown();
}

//--------------------------------------------------------------------------
// Pathing
vector<ConstVisitorAcceptorInfo> Engine::get_visitor_acceptors(
                                                  const Path& path,
                                                  unsigned path_index) const
{
  return const_cast<const Graph *>(graph.get())->get_visitor_acceptors(path,
                                                                 path_index,
                                                                 nullptr,
                                                                 nullptr);
}
vector<VisitorAcceptorInfo> Engine::get_visitor_acceptors(
                                                  const Path& path,
                                                  unsigned path_index)
{
  return graph->get_visitor_acceptors(path, path_index, nullptr, nullptr);
}

}} // namespaces
