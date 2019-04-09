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

const auto default_update_check_interval{"1"};

//------------------------------------------------------------------------
// Add an element to the graph
void Graph::add(Element *el)
{
  elements[el->id].reset(el);

  // If an acceptor assume it's disconnected until proved otherwise
  if (dynamic_cast<Acceptor *>(el))
    disconnected_acceptors.push_back(el);

  // Connect to previous element for default control targets
  if (last_element) last_element->next_element = el;
  last_element = el;
}

//------------------------------------------------------------------------
// Attach a pure Acceptor to all unbound generators remaining in the graph
// Returns whether any were attached
// Note, doesn't add to graph ordering and remembers this for reload
bool Graph::attach(Acceptor *a)
{
  if (unbound_generators.empty()) return false;

  for(auto g: unbound_generators)
    g->attach(a);
  unbound_generators.clear();
  external_acceptor = a;
  return true;
}

//------------------------------------------------------------------------
// Attach an Acceptor Element to all unbound generators remaining in the graph
// Returns whether it is an Acceptor and any were attached
bool Graph::attach(Element *el)
{
  Acceptor *acceptor = dynamic_cast<Acceptor *>(el);
  if (!acceptor || unbound_generators.empty()) return false;

  for(auto p=unbound_generators.begin(); p!=unbound_generators.end();)
  {
    Generator *g = *p;

    // Check type
    bool found = false;
    for(const auto& i: el->module->inputs)
      for(const auto& o: g->module->outputs)
        if (i.type == o.type || i.type == "any" || o.type == "any")
          found = true;

    if (found)
    {
      g->attach(acceptor, el->id);
      g->downstreams.push_back(el);
      p = unbound_generators.erase(p);
    }
    else ++p;
  }
  return true;
}

//------------------------------------------------------------------------
// Connect an element in the graph
// Uses internal state to work out how to connect it:
//   Acceptors are connected to all previous unconnected Generators
//   Controls are connected to the last non-control Element
// Throws runtime_error if it can't connect properly
void Graph::connect(Element *el)
{
  // If this is an acceptor, attach any unbound generators to it
  if (attach(el))
    disconnected_acceptors.remove(el);

  // If it's a generator, look for explicit output,
  // or add to unbound generators list
  Generator *g = dynamic_cast<Generator *>(el);
  if (g)
  {
    const auto& acceptor_id = g->get_acceptor_id();
    if (acceptor_id.empty())
    {
      // Add to unbound generators to connect to next acceptor
      unbound_generators.push_back(g);
    }
    else
    {
      Element *ae = get_element(acceptor_id);
      if (ae)
      {
        Acceptor *acceptor = dynamic_cast<Acceptor *>(ae);
        if (acceptor)
        {
          // Check type
          bool found = false;
          for(const auto& i: ae->module->inputs)
            for(const auto& o: g->module->outputs)
              if (i.type == o.type || i.type == "any" || o.type == "any")
                found = true;

          if (!found)
            throw runtime_error("Graph element " + acceptor_id
                                + " pointed to by " + el->id
                                + " has the wrong input type");

          g->attach(acceptor);
          disconnected_acceptors.remove(ae);
          el->downstreams.push_back(ae);
        }
        else
          throw runtime_error("Graph element " + acceptor_id
                              + " pointed to by " + el->id
                              + " is not an acceptor");
      }
      else throw runtime_error("No such element " + acceptor_id
                               + " pointed to by " + el->id);
    }
  }

  // If it's a control, connect it to its targets
  ControlImpl *c = dynamic_cast<ControlImpl *>(el);
  if (c)
  {
    const auto& targets = c->get_targets();
    for(const auto& it: targets)
    {
      const ControlImpl::Target& target = it.second;
      // Check properties exist and are the right type
      if (target.properties.empty())
        throw runtime_error("Control "+el->id+" has no properties\n");

      Element *target_element;
      if (it.first.empty())
      {
        // Default - connect to next one
        if (!el->next_element)
          throw runtime_error("No element to connect to control " + el->id);
        target_element = el->next_element;
      }
      else
      {
        const auto eit = elements.find(it.first);
        if (eit == elements.end())
          throw runtime_error("No such target element "+it.first
                              +" to connect to control " + el->id);
        target_element = eit->second.get();
      }

      // Attach it
      c->attach_target(it.first, target_element, el);
      el->downstreams.push_back(target_element);
    }
  }

  // Let it do any additional connection
  el->connect();
}

//------------------------------------------------------------------------
// Configure with XML, with a base directory for files
// Throws a runtime_error if configuration fails
// Public version which allows source file
void Graph::configure(const File::Directory& base_dir,
                      const XML::Element& config)
{
  // Check for load from file - if so, read it and recurse
  const auto& fn = config["file"];
  if (fn.empty())
  {
    // Configure from direct XML
    configure_internal(base_dir, config);
  }
  else
  {
    source_file = File::Path(base_dir, fn);
    file_update_check_interval =
      Time::Duration(config.get_attr("update-check-interval",
                                     default_update_check_interval));
    configure_from_source_file();
  }
}

//--------------------------------------------------------------------------
// Configure from source file, with given update check interval
void Graph::configure(const File::Path& file,
                      const Time::Duration& check_interval,
                      Acceptor *acceptor)
{
  source_file = file;
  file_update_check_interval = check_interval;
  external_acceptor = acceptor;
  configure_from_source_file();
}

//------------------------------------------------------------------------
// (re)configure from source_file
// Throws a runtime_error if configuration fails
void Graph::configure_from_source_file()
{
  Log::Streams log;
  log.summary << "Reading graph from " << source_file << endl;
  XML::Configuration fconfig(source_file.str(), log.error);
  if (!fconfig.read("graph"))
    throw(runtime_error("Can't read graph file "+source_file.str()));

  source_file_mtime = source_file.last_modified();
  const auto& root = fconfig.get_root();
  configure_internal(File::Directory(source_file.dirname()), root);
}

//------------------------------------------------------------------------
// Configure with XML, with a base directory for files
// Throws a runtime_error if configuration fails
// Internal version, doesn't allow source file redirect
void Graph::configure_internal(const File::Directory& base_dir,
                               const XML::Element& config)
{
  MT::RWWriteLock lock(mutex);
  elements.clear();
  id_serials.clear();
  disconnected_acceptors.clear();
  unbound_generators.clear();
  topological_order.clear();
  last_element = nullptr;

  // Two-phase create/connect to allow forward references

  // Create all children, but don't connect yet
  list<Element *> ordered_elements;
  for (const auto& p: config.children)
  {
    const auto& e = *p;
    if (e.name.empty()) continue;

    const auto el = engine.element_registry.create(e.name, e);
    if (!el) throw(runtime_error("No such dataflow element " + e.name));

    // Point back to us (so we're available for configure())
    el->graph = this;

    // Invent an ID if not explicitly set
    if (el->id.empty()) el->id = e.name;

    // See if it already exists - if so, add a serial number
    if (elements.find(el->id) != elements.end())
      el->id += "-"+Text::itos(++id_serials[e.name]);

    // Configure it
    el->configure(base_dir, e);

    // Add it
    add(el);
    ordered_elements.push_back(el);
  }

  // Connect all elements - note original ordering is required for shortcuts
  for(auto el: ordered_elements)
    connect(el);

  // Check for acceptors that never received any input
  for(auto& el: disconnected_acceptors)
    throw runtime_error("Element "+el->id+" has no inputs");

  // Add back any external acceptor that was given in a previous incarnation
  if (external_acceptor) attach(external_acceptor);
}

//------------------------------------------------------------------------
// Calculate topology at top level
void Graph::calculate_topology()
{
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
      e->pre_tick(td);
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
      e->tick(td);
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
      e->post_tick(td);
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
// Get the nearest particular element by ID, looking upwards in ancestors
shared_ptr<Element> Graph::get_nearest_element(const string& id)
{
  MT::RWReadLock lock(mutex);
  if (elements.find(id) != elements.end())
    return elements[id];

  if (parent)
    return parent->get_nearest_element(id);
  else
    return nullptr;
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
    // !!! Reset entire graph!
    throw runtime_error("Wholesale graph setting not yet implemented");
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
    throw runtime_error("Graph already exists");
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
      // !!! Create an element here
      throw runtime_error("Element creation not yet implemented!");
    }
  }
}

//--------------------------------------------------------------------------
// Does this require an update? (i.e. there is a new config)
bool Graph::requires_update(File::Path& file, Time::Duration& check_interval,
                            Acceptor *& acceptor)
{
  if (source_file.str().empty())
    return false;

  const auto now = Time::Stamp::now();
  const auto throttle = now - last_file_update_check
                        < file_update_check_interval;
  if (throttle)
    return false;

  const auto mtime = source_file.last_modified();
  if (!mtime)
    return false;

  if (mtime == source_file_mtime)
    return false;

  if (!source_file.length())
    return false;

  source_file_mtime = source_file.last_modified();
  file = source_file;
  check_interval = file_update_check_interval;
  acceptor = external_acceptor;
  last_file_update_check = now;
  return true;
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
