//==========================================================================
// ViGraph dataflow machines: element.cc
//
// Element implementation
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-dataflow.h"

namespace ViGraph { namespace Dataflow {

//--------------------------------------------------------------------------
// Connect an element
bool Element::connect(const string& out_name,
                      GraphElement& b, const string &in_name)
{
  auto& module = get_module();
  auto o = module.get_output(*this, out_name);
  if (!o)
  {
    Log::Error log;
    log << "Unknown output '" << out_name << "' on element "
        << get_id() << endl;
    return false;
  }
  auto inputs = b.get_connection_inputs(in_name);
  for (auto& i: inputs)
  {
    if (!o->connect(this, {&b, i}))
      return false;

#if OBTOOLS_LOG_DEBUG
    Log::Debug dlog;
    dlog << "CONNECT " << get_id() << " (" << this << "):" << out_name << " to "
         << b.get_id() << " (" << &b << "):" << in_name << endl;
#endif

    o->set_sample_rate(i->get_sample_rate());
    b.notify_connection(in_name, *this, out_name);
    update_sample_rate();
  }
  return true;
}

//--------------------------------------------------------------------------
// Get connection inputs
vector<ElementInput *> Element::get_connection_inputs(const string& name)
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
// Notify that connection has been made to input
void Element::notify_connection(const string& in_name,
                                GraphElement&, const string&)
{
  auto& module = get_module();
  auto i = module.get_input(*this, in_name);
  if (!i)
    return;
  inputs.insert(i);
}

//--------------------------------------------------------------------------
// Handle sample rate change
void Element::update_sample_rate()
{
  if (updating_sample_rate)
    return;

  auto& module = get_module();
  if (!module.has_outputs())
    return;

  updating_sample_rate = true;
  auto rate = double{};
  module.for_each_output([this, &rate](const string&, const OutputMember& om)
      {
        const auto r = om.get(*this).get_sample_rate();
        if (r > rate) // pick the highest (and also ignore unconnected)
          rate = r;
      });
  module.for_each_input([this, rate](const string&, const InputMember& im)
      {
        im.get(*this).set_sample_rate(rate);
      });
  updating_sample_rate = false;
}

//--------------------------------------------------------------------------
// Clone element
Element *Element::clone(const SetupContext&) const
{
  auto el = create_clone();
  if (!el)
    return el;

  el->set_id(get_id());

  const auto& module = get_module();
  if (module.has_settings())
  {
    module.for_each_setting([this, &el]
                            (const string&,
                             const SettingMember& setting)
    {
      setting.get(*el).set_from(setting.get(*this));
    });
  }
  if (module.has_inputs())
  {
    module.for_each_input([this, &el]
                          (const string&,
                           const InputMember& input)
    {
      input.get(*el).set_from(input.get(*this));
    });
  }

  return el;
}

//--------------------------------------------------------------------------
// Ready - check for tick readiness
bool Element::ready() const
{
  for (const auto& i: inputs)
    if (!i->ready())
      return false;
  return true;
}

//--------------------------------------------------------------------------
// Reset - prepare for tick
void Element::reset()
{
  for (auto& i: inputs)
    i->reset();
}

//--------------------------------------------------------------------------
// Pathing
vector<ConstVisitorAcceptorInfo> Element::get_visitor_acceptors(
                                                  const Path& path,
                                                  unsigned path_index,
                                                  const Graph *graph,
                                                  const Clone *clone) const
{
  if (path.reached(path_index))
    return {{get_id(), this, graph, clone}};
  if (path.type(path_index) != Path::PartType::attribute)
    return {};
  const auto& name = path.name(path_index);
  const auto& module = get_module();

  const auto s = module.get_setting(name);
  if (s)
    return {{name, s, this, graph}};

  const auto i = module.get_input(name);
  if (i)
    return {{name, i, this, graph}};

  const auto o = module.get_output(name);
  if (o)
    return {{name, o, this, graph}};

  return {};
}

vector<VisitorAcceptorInfo> Element::get_visitor_acceptors(
                                                  const Path& path,
                                                  unsigned path_index,
                                                  Graph *graph,
                                                  Clone *clone)
{
  if (path.reached(path_index))
    return {{get_id(), static_cast<VisitorAcceptor *>(this), graph, clone}};
  if (path.type(path_index) != Path::PartType::attribute)
    return {};
  const auto& name = path.name(path_index);
  const auto& module = get_module();

  const auto s = module.get_setting(name);
  if (s)
    return {{name, s, this, graph, true}};

  const auto i = module.get_input(name);
  if (i)
    return {{name, i, this, graph}};

  const auto o = module.get_output(name);
  if (o)
    return {{name, o, this, graph}};

  return {};
}

}} // namespaces
