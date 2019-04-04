//==========================================================================
// ViGraph dataflow machines: control.cc
//
// Property control implementation
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-dataflow.h"

namespace ViGraph { namespace Dataflow {

//------------------------------------------------------------------------
// Construct with XML
ControlImpl::ControlImpl(const Module *_module, const XML::Element& _config,
                         bool targets_are_optional):
  control_id(_config["id"].empty()?_config.name:_config["id"]),
  config(_config)
{
  // Simple single target
  const string& target_id = config["target"];
  if (!target_id.empty())
    targets[target_id] = Target();

  for(const auto& it: config.get_children("target"))
  {
    XML::Element& te = *it;
    const string& target_id = te["id"];
    targets[target_id] = Target();
  }

  // Create a modifiable list of properties from the module's config
  map<string, Property> properties;
  for(const auto pit: _module->controlled_properties)
    properties[pit.first] = Property(pit.second.name, pit.second.type);

  // Check for simple single property value and fix up from default
  const auto& prop = config["property"];
  bool has_explicit = false;
  if (!prop.empty() && !properties.empty())
  {
    properties.begin()->second.name = prop;
    properties.begin()->second.is_explicit = true;
    has_explicit = true;
  }

  // Check prefixed attributes
  const auto& props = config.get_attrs_with_prefix("property-");
  for(const auto& p: props)
  {
    const auto& it = properties.find(p.first);
    if (it == properties.end())
      throw runtime_error("Element "+control_id+" has no property "+p.first);
    it->second.name = p.second;
    it->second.is_explicit = true;
    has_explicit = true;
  }

  // If any explicitly set, remove all the implicit ones
  // (if you set any property names, you only get connections to the ones
  // you set, none by default)
  if (has_explicit)
  {
    for(auto it=properties.begin(); it!=properties.end();)
    {
      if (!it->second.is_explicit)
        it = properties.erase(it);
      else
        ++it;
    }
  }

  // If no targets and not optional, and we have outputs, create a default one
  if (targets.empty() && !targets_are_optional
      && !_module->controlled_properties.empty())
    targets[""] = Target();

  // Set on all targets initially
  for(auto& it: targets)
    it.second.properties = properties;

  // Now for each <target> element, override the property independently, if set
  for(const auto& it: config.get_children("target"))
  {
    XML::Element& te = *it;
    const string& target_id = te["id"];
    Target& target = targets[target_id];

    // Clear the default properties, to allow for cases where multiple targets
    // each receive different properties
    target.properties.clear();

    // Check for single property attribute
    const auto& prop = te["property"];
    if (!prop.empty())
    {
      // Reset the first (default) one using the existing type
      if (!properties.empty())
        target.properties[properties.begin()->first] =
          Property(prop, properties.begin()->second.type, true);
    }

    // Check prefixed attributes
    const auto& props = te.get_attrs_with_prefix("property-");
    for(const auto& p: props)
    {
      const auto& it = properties.find(p.first);
      if (it == properties.end())
        throw runtime_error("Element "+control_id+" has no property "+p.first);
      target.properties[p.first] = Property(p.second, it->second.type, true);
    }
  }
}

//------------------------------------------------------------------------
// Attach to a target element
void ControlImpl::attach_target(const string& prop_id,
                                Element *target_element,
                                Element *source_element)
{
  auto it = targets.find(prop_id);
  if (it == targets.end())
    throw runtime_error("No such target "+prop_id+" on control");

  auto& target = it->second;

  // Check name and type of properties
  for(auto it=target.properties.begin(); it!=target.properties.end();)
  {
    const auto& p = it->second;
    Value::Type target_type =
      target_element->get_property_type(p.name);
    if (target_type == Value::Type::invalid)
    {
      // If explicit, this is an error
      if (p.is_explicit)
        throw runtime_error("Can't connect from "+source_element->id+" to "
                            +target_element->id+"("+p.name
                            +"): no such property");
      else
      {
        // Remove it from target properties
        it = target.properties.erase(it);
        continue;
      }
    }

    // Check type
    if (p.type != target_type && p.type != Value::Type::any
        && target_type != Value::Type::any)
      throw runtime_error("Control type mismatch connecting "+
                          source_element->id+" to "
                          +target_element->id+"("+p.name
                          +"): expecting "+Value::type_str(target_type)
                          +" but got "+Value::type_str(p.type));

    // Check it's settable (they don't get to override this)
    if (target_element->module)
    {
      const auto pit = target_element->module->properties.find(p.name);
      if (pit != target_element->module->properties.end()
          && !pit->second.settable)
        throw runtime_error("Can't connect from "+source_element->id+" to "
                            +target_element->id+"("+p.name
                            +"): property not settable");

      target_element->notify_target_of(p.name);
    }

    ++it;
  }

  target.element = target_element;
}

//------------------------------------------------------------------------
// Send a value to the target using only (first) property
void ControlImpl::send(const Value& v)
{
  for(const auto& it: targets)
  {
    const Target& target = it.second;
    if (target.element && target.properties.size())
    {
      const auto& p = target.properties.begin();
      target.element->set_property(p->second.name, v);
    }
  }
}

//------------------------------------------------------------------------
// Send a named value to the target
// name is our name for it
void ControlImpl::send(const string& name, const Value& v)
{
  for(const auto& it: targets)
  {
    const Target& target = it.second;
    if (target.element)
    {
      const auto& p = target.properties.find(name);
      if (p != target.properties.end())
        target.element->set_property(p->second.name, v);  // Use their name
      else
      {
        // Look for wildcard, we can send using our name
        const auto& q = target.properties.find("");
        if (q != target.properties.end())
          target.element->set_property(name, v);  // Use our name
      }
    }
  }
}

//------------------------------------------------------------------------
// Get state as JSON, adding to the given value
void ControlImpl::add_to_json(JSON::Value& json) const
{
  if (!targets.empty())
  {
    JSON::Value& oj = json.set("outputs", JSON::Value(JSON::Value::OBJECT));
    for(const auto& tit: targets)
    {
      const auto& target = tit.second;
      for(const auto& pit: target.properties)
      {
        JSON::Value& pj = oj.set(pit.first, JSON::Value(JSON::Value::OBJECT));
        pj.set("element", target.element->id);
        pj.set("prop", pit.second.name);
      }
    }
  }
}

}} // namespaces
