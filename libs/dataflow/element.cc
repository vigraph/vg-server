//==========================================================================
// ViGraph dataflow machines: element.cc
//
// Element implementation
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-dataflow.h"

namespace ViGraph { namespace Dataflow {

//------------------------------------------------------------------------
// Get state as JSON
JSON::Value Element::get_json() const
{
  JSON::Value value(JSON::Value::Type::OBJECT);
  value.set("id", id);
  return value;
}

//------------------------------------------------------------------------
// Get type of a control property - uses module by default
Value::Type Element::get_property_type(const string& property)
{
  if (!module) return Value::Type::invalid;
  const auto pit = module->properties.find(property);
  if (pit == module->properties.end()) return Value::Type::invalid;
  return pit->second.type;
}

//------------------------------------------------------------------------
// Configure all properties from XML
// Throws a runtime_error if configuration fails
void Element::configure(const File::Directory& /*base_dir*/,
                        const XML::Element& config)
{
  configure_from_element(config, "");
  setup();
}

//------------------------------------------------------------------------
// Configure from an element, with attribute prefix
void Element::configure_from_element(const XML::Element& config,
                                     const string& prefix)
{
  for(const auto ait: config.attrs)
    configure_property(prefix+ait.first, ait.second);

  // Recurse to children with prefix 'foo.'
  for(const auto eit: config.children)
    configure_from_element(*eit, prefix+eit->name+".");
}

//------------------------------------------------------------------------
// Configure with an property name and string value
void Element::configure_property(const string& name, const string& value)
{
  const auto pit = module->properties.find(name);
  if (pit == module->properties.end()) return;  // Ignore
  const auto& prop = pit->second;

  Value v(Value::Type::invalid);
  switch (prop.type)
  {
    case Value::Type::number:
      v = Value(Text::stof(value));
      break;

    case Value::Type::text:
    case Value::Type::choice:
    case Value::Type::file:
    case Value::Type::other:
      v = Value(value);
      break;

    case Value::Type::boolean:
      v = Value(Text::stob(value)?1.0:0.0);
      break;

      // Triggers, invalid, any are ignored
    default:;
  }

  if (v.type != Value::Type::invalid)
    set_property(name, prop, v);
}

//------------------------------------------------------------------------
// Set a property (internal, with property already looked up)
void Element::set_property(const string& /*prop_name*/,
                           const Module::Property& prop,
                           const Value& v)
{
  const Module::Property::Member& member = prop.member;
  switch (v.type)
  {
    case Value::Type::number:
      if (member.d_ptr)
        this->*member.d_ptr = v.d;
      else if (member.i_ptr)
        this->*member.i_ptr = static_cast<int>(round(v.d));
      else if (member.b_ptr)
        this->*member.b_ptr = v.d?true:false;
      else if (member.set_d)
        (this->*member.set_d)(v.d);
      else if (member.set_i)
        (this->*member.set_i)(static_cast<int>(round(v.d)));
      else if (member.set_b)
        (this->*member.set_b)(v.d?1.0:0.0);
      // Fail once all modules have these!!!
      // else
        //        throw runtime_error("No member pointers for property "+prop_name
        //                    +" in element "+id);
      break;

    case Value::Type::text:
      if (member.s_ptr)
        this->*member.s_ptr = v.s;
      if (member.set_s)
        (this->*member.set_s)(v.s);
      // !!! error handle as above
      break;

    case Value::Type::trigger:
      if (member.trigger)
        (this->*member.trigger)();
      break;

    default:;
  }
}

//------------------------------------------------------------------------
// Set a property (external)
void Element::set_property(const string& prop_name,
                           const SetParams& sp)
{
  const auto pit = module->properties.find(prop_name);
  if (pit == module->properties.end())
    throw runtime_error("No such property "+prop_name+" on element "+id);
  set_property(prop_name, pit->second, sp.v);

  // Action changes
  // !!! Later optimise to only call once for sets of updates?
  update();
}

}} // namespaces
