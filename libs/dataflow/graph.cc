//==========================================================================
// ViGraph dataflow machines: graph.cc
//
// Graph structure implementation
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-dataflow.h"
#include "ot-log.h"

namespace ViGraph { namespace Dataflow {

//------------------------------------------------------------------------
// Add an element to the graph (testing)
void Graph::add(Element *el)
{
  elements[el->id].reset(el);
}

//------------------------------------------------------------------------
// Create an element
Element *Graph::create_element(const string& type, const string& id)
{
  const auto el = engine.create(type);
  if (!el) throw runtime_error("No such dataflow element " + type);
  el->id = id;

  // Point back to us
  el->graph = this;

  return el;
}

//------------------------------------------------------------------------
// Final setup for elements and calculate topology
void Graph::setup()
{
  for(const auto& it: elements)
    it.second->setup();

  Element::Topology topo;
  calculate_topology(topo);
}

//------------------------------------------------------------------------
// Calculate topology (see Element::calculate_topology)
void Graph::calculate_topology(Element::Topology& topo,
                               Element *owner)
{
  // Algorithm:  Each graph level asks its children (which may be
  // subgraphs) for a list of router senders and receivers, which it
  // then combines to add to the senders' downstreams ('before' dependencies)

  // The sub-graph holding elements <graph>, <clone>, <selector> will pass
  // themselves as 'owner', and we proxy our children's send/receive tags as
  // the owner, to create the dependencies in the level above.

  // Our internal topology
  Element::Topology our_topo;

  // Pass down to elements
  for(const auto& it: elements)
    it.second->calculate_topology(our_topo);

  // For each sender, find all receivers for its tag, and add as downstreams
  for(const auto& sit: our_topo.router_senders)
  {
    const auto& sender_tag = sit.first;
    const auto& senders = sit.second;

    const auto rit = our_topo.router_receivers.find(sender_tag);
    if (rit != our_topo.router_receivers.end())
    {
      const auto& receivers = rit->second;

      // Attach all receivers as downstreams of senders
      for(auto sender: senders)
        sender->downstreams.insert(sender->downstreams.end(),
                                   receivers.begin(), receivers.end());
    }
  }

  // Calculate our own topological order based on existing and newly-added
  // element downstreams
  generate_topological_order();

  if (owner) // not at top level
  {
    // Add all senders as receivers to parent's topology, proxying as our owner
    for(const auto& sit: our_topo.router_senders)
      topo.router_senders[sit.first].push_back(owner);
    for(const auto& sit: our_topo.router_receivers)
      topo.router_receivers[sit.first].push_back(owner);
  }
}

//------------------------------------------------------------------------
// Recursive topological sort.  If not already visited, recurses to all
// unvisited downstream elements and then adds itself to front of
// topological_order.
void Graph::toposort(Element *e, set<Element *>& visited)
{
  // Insert into visited set, check if added new
  // Note doing this before the recursion breaks loops
  if (visited.insert(e).second)
  {
    for(const auto de: e->downstreams)
      toposort(de, visited);
    topological_order.push_front(e);
  }
}

//------------------------------------------------------------------------
// Generate topological order - ordered list of elements which ensures
// a precursor (upstream) element is ticked before its dependents
// (downstreams)
void Graph::generate_topological_order()
{
  set<Element *> visited;
  topological_order.clear();

  // Start with elements in reverse order - because it is stacked this retains
  // the order of 0-in-degree elements
  for(auto it = elements.rbegin(); it != elements.rend(); ++it)
    toposort(it->second.get(), visited);
}

//------------------------------------------------------------------------
// Enable all elements
void Graph::enable()
{
  MT::RWReadLock lock(mutex);
  for(const auto e: topological_order)
    e->enable();
  is_enabled = true;
}

//------------------------------------------------------------------------
// Disable all elements
void Graph::disable()
{
  MT::RWReadLock lock(mutex);
  for(const auto e: topological_order)
    e->disable();
  is_enabled = false;
}

//------------------------------------------------------------------------
// Pre-tick all elements in topological order
void Graph::pre_tick(const TickData& td)
{
  MT::RWReadLock lock(mutex);
  for(const auto e: topological_order)
  {
    try
    {
      if (sample_rate != td.sample_rate)
      {
        auto ntd = td;
        ntd.sample_rate = sample_rate;
        e->pre_tick(ntd);
      }
      else
      {
        e->pre_tick(td);
      }
    }
    catch (const runtime_error& re)
    {
      Log::Error log;
      log << "Pre-tick failed for " << e->id << ": " << re.what() << endl;
    }
  }
}

//------------------------------------------------------------------------
// Tick all elements in topological order
void Graph::tick(const TickData& td)
{
  MT::RWReadLock lock(mutex);
  for(const auto e: topological_order)
  {
    try
    {
      if (sample_rate != td.sample_rate)
      {
        auto ntd = td;
        ntd.sample_rate = sample_rate;
        e->tick(ntd);
      }
      else
      {
        e->tick(td);
      }
    }
    catch (const runtime_error& re)
    {
      Log::Error log;
      log << "Tick failed for " << e->id << ": " << re.what() << endl;
    }
  }
}

//------------------------------------------------------------------------
// Post-tick all elements
void Graph::post_tick(const TickData& td)
{
  MT::RWReadLock lock(mutex);
  for(const auto e: topological_order)
  {
    try
    {
      if (sample_rate != td.sample_rate)
      {
        auto ntd = td;
        ntd.sample_rate = sample_rate;
        e->post_tick(ntd);
      }
      else
      {
        e->post_tick(td);
      }
    }
    catch (const runtime_error& re)
    {
      Log::Error log;
      log << "Post-tick failed for " << e->id << ": " << re.what() << endl;
    }
  }
}

//------------------------------------------------------------------------
// Get a particular element by ID
Element *Graph::get_element(const string& id)
{
  MT::RWReadLock lock(mutex);
  if (elements.find(id) != elements.end())
    return elements[id].get();
  else
    return nullptr;
}

//------------------------------------------------------------------------
// Get the nearest particular element by section and type, looking upwards
// in ancestors
shared_ptr<Element> Graph::get_nearest_element(const string& section,
                                               const string& type)
{
  MT::RWReadLock lock(mutex);
  // !!! Linear search!
  for(const auto& it: elements)
    if (it.second->module->section == section
     && it.second->module->id == type)
      return it.second;

  if (parent)
    return parent->get_nearest_element(section, type);
  else
    return nullptr;
}

//------------------------------------------------------------------------
// Send data up to be sent on by owning element in the level above
void Graph::send_up(DataPtr data)
{
  if (send_up_function) send_up_function(data);
}

//------------------------------------------------------------------------
// Get state as JSON array of elements
// Path is an XPath-like list of subgraph IDs and leaf element, or empty
// for entire graph
JSON::Value Graph::get_json(const string& path) const
{
  MT::RWReadLock lock(mutex);
  if (path.empty())
  {
    JSON::Value value(JSON::Value::Type::ARRAY);
    for(const auto e: topological_order)
      value.add(e->get_json(path));
    return value;
  }
  else
  {
    // Split path and use first (or only) as ID, pass rest (or empty) down
    vector<string> bits = Text::split(path, '/', false, 2);
    const auto it = elements.find(bits[0]);
    if (it == elements.end())
      throw runtime_error("No such sub-element "+bits[0]+" in graph");

    // Return bare value (or INVALID) up, undecorated
    return it->second->get_json(bits.size()>1 ? bits[1] : "");
  }
}

//------------------------------------------------------------------------
// Set state from JSON
// path is a path/to/leaf/prop - can set any intermediate level too
void Graph::set_json(const string& path, const JSON::Value& value)
{
  if (path.empty())
  {
    // Treat set (POST) and add (PUT) the same
    add_json(path, value);
  }
  else
  {
    // Pass down to individual element
    vector<string> bits = Text::split(path, '/', false, 2);

    // Find first part or leaf element
    const auto& it = elements.find(bits[0]);
    if (it == elements.end())
      throw runtime_error("No such element "+bits[0]+" in graph");

    it->second->set_json(bits.size()>1 ? bits[1] : "", value);
  }
}

//------------------------------------------------------------------------
// Add a new element from JSON
// path is a path/to/leaf
void Graph::add_json(const string& path, const JSON::Value& value)
{
  if (path.empty())
  {
    // Top-level replacement - destroy any existing, then recreate
    if (!parent) shutdown();

    if (value.type != JSON::Value::ARRAY)
      throw runtime_error("Whole graph setting needs a JSON array");

    // If any of this fails, clean up and rethrow
    try
    {
      // Create elements first, recursively
      for(const auto& v: value.a)
      {
        const auto& id = v["id"].as_str();
        if (id.empty()) throw runtime_error("Graph element requires an 'id'");
        add_element_from_json(id, v); // Note no setup(), we do it after config
      }

      // Then configure and connect them
      for(const auto& v: value.a)
      {
        const auto& id = v["id"].as_str();
        set_json(id, v);
      }

      // Do final setup and topology
      setup();

      // Enable if top level (this is recursive, so
      // otherwise we would do it multiple times)
      if (!parent) enable();
    }
    catch (const runtime_error& e)
    {
      if (!parent) shutdown();
      throw e;
    }
  }
  else
  {
    vector<string> bits = Text::split(path, '/', false, 2);
    if (bits.size() > 1)
    {
      // Pass down to subgraph/element
      const auto& it = elements.find(bits[0]);
      if (it == elements.end())
        throw runtime_error("No such element "+bits[0]+" in graph");
      it->second->add_json(bits[1], value);
    }
    else
    {
      // Add it and setup as default
      add_element_from_json(path, value)->setup();
    }
  }
}

//------------------------------------------------------------------------
// Add a new element with JSON config
// Internal only - doesn't do setup()
// Returns created element
Element *Graph::add_element_from_json(const string& id,
                                      const JSON::Value& value)
{
  // Check it doesn't already exist
  if (elements.find(id) != elements.end())
    throw runtime_error("Element '"+id+"' already exists");

  // Get type
  if (value.type != JSON::Value::OBJECT)
    throw runtime_error("Element creation needs a JSON object with 'type'");
  const auto& type = value["type"].as_str();
  if (type.empty())
    throw runtime_error("Element creation needs a 'type'");

  // Create an element
  auto el = create_element(type, id);
  elements[id].reset(el);

  // Add to end of topological order for now, it will be updated once
  // connected
  topological_order.push_back(el);

  Log::Detail log;
  log << "Created " << type << " '" << id << "'\n";
  return el;
}

//------------------------------------------------------------------------
// Delete an item (from REST)
// path is a path/to/leaf
void Graph::delete_item(const string& path)
{
  if (path.empty())
    throw runtime_error("Can't delete in whole graph");

  vector<string> bits = Text::split(path, '/', false, 2);
  const auto& it = elements.find(bits[0]);
  if (it == elements.end())
    throw runtime_error("No such element "+bits[0]+" in graph");
  auto el = it->second.get();

  if (bits.size() > 1)
  {
    // Pass down to subgraph/element
    el->delete_item(bits[1]);
  }
  else
  {
    // Disconnect from (e.g.) router etc.
    el->disable();

    // Disconnect from other elements in the graph
    for(const auto eit: elements)
      eit.second->disconnect(el);

    // Delete it
    topological_order.remove(el);
    el->shutdown();
    elements.erase(it);
  }

  // Recalculate topological order
  generate_topological_order();
}

//------------------------------------------------------------------------
// Shutdown all elements
void Graph::shutdown()
{
  disable();

  MT::RWWriteLock lock(mutex);
  for(const auto it: elements)
    it.second->shutdown();

  // Remove all elements before modules unloaded
  elements.clear();
  topological_order.clear();
}



}} // namespaces
