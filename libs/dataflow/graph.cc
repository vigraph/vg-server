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

//--------------------------------------------------------------------------
// Add an element to the graph (testing)
void Graph::add(GraphElement *el)
{
  elements[el->get_id()].reset(el);
}

//--------------------------------------------------------------------------
// Remove an element
void Graph::remove(const string& id)
{
  remove_input_pin(id);
  remove_output_pin(id);
  auto it = elements.find(id);
  if (it == elements.end())
    throw(runtime_error{"Element not found: " + id});
  elements.erase(it);
}


//--------------------------------------------------------------------------
// Connect an element
bool Graph::connect(const string& out_name,
                    GraphElement& b, const string& in_name)
{
  // Check if connecting an output pin
  auto oit = output_pins.find(out_name);
  if (oit == output_pins.end())
    return false;

  const auto& pin_info = oit->second;
  auto output_pin = get_element(pin_info.element);
  if (!output_pin)
    return false;

  // Pass connection on to the pin
  return output_pin->connect(pin_info.connection, b, in_name);
}

//--------------------------------------------------------------------------
// Get connection inputs
vector<ElementInput *> Graph::get_connection_inputs(const string& name)
{
  auto& module = get_module();
  auto i = module.get_input(*this, name);
  if (!i)
  {
    Log::Error log;
    log << "Unknown input '" << name << "' on element " << get_id() << endl;
    return {};
  }

  return vector<ElementInput *>{i};
}

//--------------------------------------------------------------------------
// Clone
Graph *Graph::clone(const SetupContext& context) const
{
  auto g = new Graph{GraphModule{}};
  g->set_id(get_id());

  // Elements
  auto pairs = vector<pair<GraphElement *, GraphElement *>>{};
  for (const auto& el: elements)
  {
    auto c = el.second->clone(context);
    c->setup(context);
    g->elements.emplace(el.first, c);
    pairs.emplace_back(el.second.get(), c);
  }

  // Input pins
  for (const auto &ip: input_pins)
  {
    g->add_input_pin(ip.first, ip.second.element, ip.second.connection);
  }

  // Output pins
  for (const auto &op: output_pins)
  {
    g->add_output_pin(op.first, op.second.element, op.second.connection);
  }

  // Element connections - internal only
  for (auto& els: pairs)
  {
    auto orig = els.first;
    auto clone = els.second;
    auto& m = orig->get_module();
    if (m.has_outputs())
    {
      m.for_each_output([&orig, &clone, &g, this]
                        (const string& id, const OutputMember& output)
      {
        auto& op = output.get(*orig);
        const auto& conns = op.get_connections();
        for (const auto& conn: conns)
        {
          const auto& eid = conn.element->get_id();
          if (this->get_element(eid) != conn.element)
            continue; // Connection to element outside graph
          auto e = g->get_element(eid);
          if (!e)
            continue;
          const auto& emodule = conn.element->get_module();
          const auto& einput = emodule.get_input_id(*conn.element, *conn.input);
          clone->connect(id, *e, einput);
        }
      });
    }
  }

  // Connect inputs to graph
  auto& module = get_module();
  auto& gmodule = g->get_module();
  for (const auto &ip: input_pins)
  {
    auto im = module.get_input(ip.first);
    auto gim = gmodule.get_input(ip.first);
    if (im && gim)
    {
      auto& i = im->get(*this);
      auto& gi = gim->get(*g);
      const auto conns = i.get_connections();
      for (auto& conn: conns)
      {
        // Find the matching reverse connection in order to use the element
        // that sees us as - which should be the clone
        const auto revconns = conn.output->get_connections();
        for (auto &rconn: revconns)
        {
          if (rconn.input == &i)
          {
            conn.output->connect(conn.element, {rconn.element, &gi});
            break;
          }
        }
      }
    }
  }

  // Connect outputs to graph
  for (const auto &op: output_pins)
  {
    auto om = module.get_output(op.first);
    if (om)
    {
      auto& o = om->get(*this);
      const auto conns = o.get_connections();
      for (auto& conn: conns)
      {
        const auto& emodule = conn.element->get_module();
        const auto& einput = emodule.get_input_id(*conn.element,
                                                  *conn.input);
        g->connect(op.first, *conn.element, einput);
      }
    }
  }

  return g;
}

//--------------------------------------------------------------------------
// Add input pin
void Graph::add_input_pin(const string& id,
                          const string& element, const string& input)
{
  auto pin = get_element(element);
  if (!pin)
  {
    Log::Error log;
    log << "Bad input pin element " << element << endl;
    return;
  }
  input_pins.emplace(id, PinInfo{element, input});
  module.inputs.emplace(id, *pin);
}

//--------------------------------------------------------------------------
// Remove input pin
void Graph::remove_input_pin(const string& id)
{
  input_pins.erase(id);
  module.inputs.erase(id);
}

//--------------------------------------------------------------------------
// Add output pin
void Graph::add_output_pin(const string& id,
                          const string& element, const string& output)
{
  auto pin = get_element(element);
  if (!pin)
  {
    Log::Error log;
    log << "Bad output pin element " << element << endl;
    return;
  }
  output_pins.emplace(id, PinInfo{element, output});
  module.outputs.emplace(id, *pin);
}

//--------------------------------------------------------------------------
// Remove output pin
void Graph::remove_output_pin(const string& id)
{
  auto it = output_pins.find(id);
  if (it == output_pins.end())
    return;
  auto el = elements.find(id);
  if (el != elements.end())
  {
    auto& m = el->second->get_module();
    auto o = m.get_output(*el->second, it->second.connection);
    if (o)
      o->disconnect();
  }
  output_pins.erase(it);
  module.outputs.erase(id);
}

//------------------------------------------------------------------------
// Final setup for elements and calculate topology
void Graph::setup(const SetupContext& context)
{
  for(const auto& it: elements)
    it.second->setup(context);
}

//------------------------------------------------------------------------
// Get a particular element by ID
GraphElement *Graph::get_element(const string& id) const
{
  MT::RWReadLock lock(mutex);
  const auto it = elements.find(id);
  if (it != elements.end())
    return it->second.get();
  else
    return nullptr;
}

//------------------------------------------------------------------------
// Remove all elements
void Graph::clear()
{
  MT::RWWriteLock lock(mutex);
  elements.clear();
}

//--------------------------------------------------------------------------
// Pathing
vector<ConstVisitorAcceptorInfo> Graph::get_visitor_acceptors(
                                                  const Path& path,
                                                  unsigned path_index,
                                                  const Graph *graph,
                                                  const Clone *clone) const
{
  if (path.reached(path_index))
    return {{get_id(), this, graph, clone}};

  const auto& n = path.name(path_index);

  if (path.type(path_index) == Path::PartType::attribute)
  {
    const auto& module = get_module();

    const auto s = module.get_setting(n);
    if (s)
      return {{n, s, this, graph}};

    const auto i = module.get_input(n);
    if (i)
      return {{n, i, this, graph}};

    const auto o = module.get_output(n);
    if (o)
      return {{n, o, this, graph}};

    throw(runtime_error{"Attribute " + n + " not found"});
  }

  auto it = elements.find(n);
  if (it == elements.end())
    throw(runtime_error{"Element " + n + " not found"});
  return static_cast<const GraphElement *>(it->second.get())
                            ->get_visitor_acceptors(path, path_index + 1,
                                                    this, clone);
}

vector<VisitorAcceptorInfo> Graph::get_visitor_acceptors(
                                                  const Path& path,
                                                  unsigned path_index,
                                                  Graph *graph,
                                                  Clone *clone)
{
  if (path.reached(path_index))
    return {{get_id(), static_cast<VisitorAcceptor *>(this), graph, clone}};

  const auto& n = path.name(path_index);

  if (path.type(path_index) == Path::PartType::attribute)
  {
    const auto& module = get_module();

    const auto s = module.get_setting(n);
    if (s)
      return {{n, s, this, graph, true}};

    const auto i = module.get_input(n);
    if (i)
      return {{n, i, this, graph}};

    const auto o = module.get_output(n);
    if (o)
      return {{n, o, this, graph}};

    return {{n, this, graph}}; // create pin
  }

  auto it = elements.find(n);
  if (it == elements.end())
  {
    if (path.reached(path_index + 1))
      return {{path.leaf(), this, clone}};
    throw(runtime_error{"Element " + n + " not found"});
  }
  return it->second->get_visitor_acceptors(path, path_index + 1,
                                           this, clone);
}

}} // namespaces
