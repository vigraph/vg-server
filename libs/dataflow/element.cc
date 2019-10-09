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
  auto& bmodule = b.get_module();
  auto i = bmodule.get_input(b, in_name);
  if (!i)
  {
    Log::Error log;
    log << "Unknown input '" << out_name << "' on element "
        << b.get_id() << endl;
    return false;
  }
  if (!o->connect({&b, i}))
    return false;
  b.notify_connection(in_name, *this, out_name);
  return true;
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
// Clone element
Element *Element::clone() const
{
  auto el = create_clone();
  if (!el)
    return el;

  el->set_id(get_id());
  // !!! TODO: clone settings
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
// Accept visitors
template<class E, class V>
void accept_visitor(E& element, V& visitor,
                    const Path& path, unsigned path_index)
{
  if (path.reached(path_index))
  {
    visitor.visit(element, path, path_index);

    auto& module = element.get_module();
    if (module.has_settings())
    {
      module.for_each_setting([&element, &visitor, &path, &path_index]
                              (const string& id,
                               const SettingMember& setting)
      {
        auto sv = visitor.get_element_setting_visitor(id);
        if (sv)
          setting.accept(*sv, path, ++path_index, element);
      });
    }

    if (module.has_inputs())
    {
      module.for_each_input([&element, &visitor, &path, &path_index]
                            (const string& id,
                             const InputMember& input)
      {
        auto iv = visitor.get_element_input_visitor(id);
        if (iv)
          input.accept(*iv, path, ++path_index, element);
      });
    }

    if (module.has_outputs())
    {
      module.for_each_output([&element, &visitor, &path, &path_index]
                             (const string& id,
                              const OutputMember& output)
      {
        auto ov = visitor.get_element_output_visitor(id);
        if (ov)
          output.accept(*ov, path, ++path_index, element);
      });
    }
  }
}

void Element::accept(ReadVisitor& visitor,
                     const Path& path, unsigned path_index) const
{
  accept_visitor(*this, visitor, path, path_index);
}

void Element::accept(WriteVisitor& visitor,
                     const Path& path, unsigned path_index)
{
  accept_visitor(*this, visitor, path, path_index);
}

}} // namespaces
