//==========================================================================
// ViGraph dataflow machines: engine.cc
//
// Global dataflow engine implementation
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-dataflow.h"
#include "ot-log.h"
#include <algorithm>

namespace ViGraph { namespace Dataflow {

//--------------------------------------------------------------------------
// Create an element with the given type
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
  return nullptr;
}

//--------------------------------------------------------------------------
// Setup an element
void Engine::setup(GraphElement& element) const
{
  element.setup(context);
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
  // Lock for whole tick process
  // Leaving the timing / tick skipping outside of the lock can result
  // in skipping ticks introduce jitter into real time / tick time differences
  // That affects things like MIDI output which rate paces using tick time
  // against clock time
  MT::RWReadLock lock(graph_mutex);
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
      if (threads.empty())
        serial_tick_elements(td);
      else
        parallel_tick_elements(td);
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
// Handle deadlock
void Engine::handle_deadlock(const vector<Element *>::const_iterator& begin,
                             const vector<Element *>::const_iterator& end)
{
  Log::Error elog;
  elog << "Deadlock detected in tick. Remaining elements:" << endl;
  for_each(begin, end, [&](Element * el)
  {
    elog << "  " << el->get_id() << endl;
  });
}

//--------------------------------------------------------------------------
// Serial tick of elements
void Engine::serial_tick_elements(const TickData& td)
{
  auto ticked = 0u;
  auto last_count = ticked;
  const auto to_tick = tick_elements.size();
  while (ticked < to_tick)
  {
    for (auto i = ticked; i < to_tick; ++i)
    {
      if (tick_elements[i]->ready())
      {
        tick_elements[i]->tick(td);
        iter_swap(tick_elements.begin() + i, tick_elements.begin() + ticked);
        ++ticked;
      }
    }
    if (ticked == last_count)
    {
      handle_deadlock(tick_elements.begin() + ticked, tick_elements.end());
      break;
    }

    last_count = ticked;
  }

  // Reset all
  for (auto it: tick_elements)
    it->reset();
}

//--------------------------------------------------------------------------
// Set the number of threads
void Engine::set_threads(unsigned nthreads)
{
  parallel_state.shutdown = true;
  for (auto& go: parallel_state.go)
    go.signal();
  for (auto& t: threads)
    t.join();
  threads.clear();
  parallel_state.go.clear();
  parallel_state.complete.clear();
  parallel_state.complete_threads.clear();

  parallel_state.shutdown = false;
  if (nthreads > 1)
  {
    parallel_state.go.resize(nthreads);
    parallel_state.complete_threads.resize(nthreads);
    while (threads.size() < nthreads)
    {
      const auto n = threads.size();
      threads.emplace_back([&, n]()
      {
        while (true)
        {
          parallel_state.go[n].wait();
          parallel_state.go[n].clear();
          if (parallel_state.shutdown)
            return;

          auto nels = tick_elements.size();
          while (true)
          {
            Element *el = nullptr;
            auto ticked = 0u;
            {
              MT::Lock lock{parallel_state.tick_elements_mutex};
              for (auto i = parallel_state.ticked; i < nels; ++i)
              {
                if (parallel_state.tick_elements[i]->ready())
                {
                  el = parallel_state.tick_elements[i];
                  iter_swap(parallel_state.tick_elements.begin() + i,
                            parallel_state.tick_elements.begin() +
                            parallel_state.ticked);
                  ticked = ++parallel_state.ticked;
                  break;
                }
              }
            }
            if (!el)
            {
              MT::Lock lock{parallel_state.complete_threads_mutex};
              parallel_state.complete_threads[n] = true;
              if (find(begin(parallel_state.complete_threads),
                       end(parallel_state.complete_threads), false)
                  == end(parallel_state.complete_threads))
                parallel_state.complete.signal();
              break;
            }

            el->tick(parallel_state.td);

            MT::Lock lock{parallel_state.complete_threads_mutex};
            if (ticked < nels)
            {
              for (auto g = 0u; g < parallel_state.go.size(); ++g)
              {
                if (g != n && parallel_state.complete_threads[g])
                {
                  parallel_state.complete_threads[g] = false;
                  parallel_state.go[g].signal();
                }
              }
            }
          }
        }
      });
    }
  }
}


//--------------------------------------------------------------------------
// Parallel tick of elements
void Engine::parallel_tick_elements(const TickData& td)
{
  parallel_state.td = td;
  parallel_state.ticked = 0;
  for (auto& go: parallel_state.go)
    go.signal();
  parallel_state.complete.wait();
  parallel_state.complete.clear();

  if (parallel_state.ticked < tick_elements.size())
    handle_deadlock(tick_elements.begin() + parallel_state.ticked,
                    tick_elements.end());

  // Reset all
  for (auto it: tick_elements)
    it->reset();
  for (auto&& c: parallel_state.complete_threads)
    c = false;
}

//--------------------------------------------------------------------------
// Shut down the graph
void Engine::shutdown()
{
  // Shut down graph
  MT::RWWriteLock lock(graph_mutex);
  graph->shutdown();
  parallel_state.shutdown = true;
  for (auto& go: parallel_state.go)
    go.signal();
  for (auto& t: threads)
    t.join();
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
