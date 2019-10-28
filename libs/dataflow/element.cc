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
// Handle sample rate change
void Element::update_sample_rate()
{
  auto& module = get_module();
  if (!module.has_outputs())
    return;
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
}

//--------------------------------------------------------------------------
// Clone element
Element *Element::clone() const
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
// Accept visitors
void Element::accept(ReadVisitor& visitor,
                     const Path& path, unsigned path_index) const
{
  if (path.reached(path_index))
  {
    visitor.visit(*this, path, path_index);

    auto& module = get_module();
    if (module.has_settings())
    {
      module.for_each_setting([this, &visitor, &path, &path_index]
                              (const string& id,
                               const SettingMember& setting)
      {
        auto sv = visitor.get_element_setting_visitor(*this, id,
                                                      path, path_index);
        if (sv)
          setting.accept(*sv, path, path_index + 1, *this);
      });
    }

    if (module.has_inputs())
    {
      module.for_each_input([this, &visitor, &path, &path_index]
                            (const string& id,
                             const InputMember& input)
      {
        auto iv = visitor.get_element_input_visitor(*this, id,
                                                    path, path_index);
        if (iv)
          input.accept(*iv, path, path_index + 1, *this);
      });
    }

    if (module.has_outputs())
    {
      module.for_each_output([this, &visitor, &path, &path_index]
                             (const string& id,
                              const OutputMember& output)
      {
        auto ov = visitor.get_element_output_visitor(*this, id,
                                                     path, path_index);
        if (ov)
          output.accept(*ov, path, path_index + 1, *this);
      });
    }
  }
  else
  {
    auto part = path.get(path_index);

    switch (part.type)
    {
      case Path::PartType::attribute:
        {
          auto& module = get_module();
          auto s = module.get_setting(part.name);
          if (s)
          {
            auto sv = visitor.get_element_setting_visitor(*this, part.name,
                                                          path, path_index);
            if (sv)
              s->accept(*sv, path, path_index + 1, *this);
          }
          else
          {
            auto i = module.get_input(part.name);
            if (i)
            {
              auto iv = visitor.get_element_input_visitor(*this, part.name,
                                                          path, path_index);
              if (iv)
                i->accept(*iv, path, path_index + 1, *this);
            }
            else
            {
              auto o = module.get_output(part.name);
              if (o)
              {
                auto ov = visitor.get_element_output_visitor(*this, part.name,
                                                             path, path_index);
                if (ov)
                  o->accept(*ov, path, path_index + 1, *this);
              }
              else
              {
                throw(runtime_error{"Attribute not found: " + part.name});
              }
            }
          }
        }
        break;
      case Path::PartType::element:
        {
          auto& module = get_module();
          throw(runtime_error{module.get_full_type() + "has no sub-elements"});
        }
        break;
      default:
        break;
    }
  }
}

void Element::accept(WriteVisitor& visitor,
                     const Path& path, unsigned path_index)
{
  if (path.reached(path_index))
  {
    if (!visitor.visit(*this, path, path_index))
      return; // element does not exist

    auto& module = get_module();
    if (module.has_settings())
    {
      module.for_each_setting([this, &visitor, &path, &path_index]
                              (const string& id,
                               const SettingMember& setting)
      {
        auto sv = visitor.get_element_setting_visitor(*this, id,
                                                      path, path_index);
        if (sv)
          setting.accept(*sv, path, path_index + 1, *this);
      });
    }
    setup();

    if (module.has_inputs())
    {
      module.for_each_input([this, &visitor, &path, &path_index]
                            (const string& id,
                             const InputMember& input)
      {
        auto iv = visitor.get_element_input_visitor(*this, id,
                                                    path, path_index);
        if (iv)
          input.accept(*iv, path, path_index + 1, *this);
      });
    }

    if (module.has_outputs())
    {
      module.for_each_output([this, &visitor, &path, &path_index]
                             (const string& id,
                              const OutputMember& output)
      {
        auto ov = visitor.get_element_output_visitor(*this, id,
                                                     path, path_index);
        if (ov)
          output.accept(*ov, path, path_index + 1, *this);
      });
    }
  }
  else
  {
    auto part = path.get(path_index);

    switch (part.type)
    {
      case Path::PartType::attribute:
        {
          auto& module = get_module();
          auto s = module.get_setting(part.name);
          if (s)
          {
            auto sv = visitor.get_element_setting_visitor(*this, part.name,
                                                          path, path_index);
            if (sv)
            {
              s->accept(*sv, path, path_index + 1, *this);
              setup();
            }
          }
          else
          {
            auto i = module.get_input(part.name);
            if (i)
            {
              auto iv = visitor.get_element_input_visitor(*this, part.name,
                                                          path, path_index);
              if (iv)
                i->accept(*iv, path, path_index + 1, *this);
            }
            else
            {
              auto o = module.get_output(part.name);
              if (o)
              {
                auto ov = visitor.get_element_output_visitor(*this, part.name,
                                                             path, path_index);
                if (ov)
                  o->accept(*ov, path, path_index + 1, *this);
              }
              else
              {
                throw(runtime_error{"Attribute not found: " + part.name});
              }
            }
          }
        }
        break;
      case Path::PartType::element:
        {
          auto& module = get_module();
          throw(runtime_error{module.get_full_type() + "has no sub-elements"});
        }
        break;
      default:
        break;
    }
  }
}

}} // namespaces
