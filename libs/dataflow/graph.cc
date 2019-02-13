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

  for(auto g: unbound_generators)
  {
    g->attach(acceptor);
    g->downstreams.push_back(el);
  }
  unbound_generators.clear();
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
  Control *c = dynamic_cast<Control *>(el);
  if (c)
  {
    const auto& targets = c->get_targets();
    for(const auto& it: targets)
    {
      const Control::Target& target = it.second;
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

      // Check name and type of properties
      for(const auto& p: target.properties)
      {
        Value::Type target_type =
          target_element->get_property_type(p.second.name);
        if (target_type == Value::Type::invalid)
          continue;  // Ignore for now - later make explicit if it's optional?

        if (p.second.type != target_type && p.second.type != Value::Type::any
            && target_type != Value::Type::any)
          throw runtime_error("Control type mismatch connecting "+el->id+" to "
                              +target_element->id+"("+p.second.name
                              +"): expecting "+Value::type_str(target_type)
                              +" but got "+Value::type_str(p.second.type));

        // Check it's settable (they don't get to override this)
        if (target_element->module)
        {
          const auto pit =
            target_element->module->properties.find(p.second.name);

          // If this property doesn't exist in the module definition but
          // the target element didn't return Type::invalid above, then
          // it must be dynamically created, so we'll assume it's settable
          // (e.g. <wrap>, <limit>)
          if (pit != target_element->module->properties.end()
              && !pit->second.settable)
            throw runtime_error("Can't connect "+el->id+" to "
                                +target_element->id+"("+p.second.name
                                +"): property not settable");

          target_element->notify_target_of(el, p.second.name);
        }
      }

      // Attach it
      c->attach_target(it.first, target_element);
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
  for(auto el: disconnected_acceptors)
    throw runtime_error("Element "+el->id+" has no inputs");

  // Add back any external acceptor that was given in a previous incarnation
  if (external_acceptor) attach(external_acceptor);

  // Generate the topological order from the graph
  generate_topological_order();
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
  // Check for file update
  Time::Stamp now = Time::Stamp::now();
  if (!source_file.str().empty()
      && now-last_file_update_check >= file_update_check_interval)
  {
    time_t mtime = source_file.last_modified();
    if (mtime != source_file_mtime)
    {
      bool was_enabled = is_enabled;
      shutdown();

      try
      {
        configure_from_source_file();
      }
      catch (runtime_error e)
      {
        Log::Error log;
        log << "Graph reload failed: " << e.what() << endl;
      }

      if (was_enabled) enable();  // re-enable if it was previously
    }

    last_file_update_check = now;
  }

  MT::RWReadLock lock(mutex);
  for(const auto e: topological_order)
    e->pre_tick(td);
}

//------------------------------------------------------------------------
// Tick all elements in topological order
void Graph::tick(const TickData& td)
{
  MT::RWReadLock lock(mutex);
  for(const auto e: topological_order)
    e->tick(td);
}

//------------------------------------------------------------------------
// Post-tick all elements
void Graph::post_tick(const TickData& td)
{
  MT::RWReadLock lock(mutex);
  for(const auto e: topological_order)
    e->post_tick(td);
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
