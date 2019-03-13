//==========================================================================
// ViGraph dataflow machines: control.cc
//
// Property control implementation
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-dataflow.h"

namespace ViGraph { namespace Dataflow {

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
  if (!prop.empty() && !properties.empty())
    properties.begin()->second.name = prop;

  // Check prefixed attributes
  const auto& props = config.get_attrs_with_prefix("property-");
  for(const auto& p: props)
  {
    const auto& it = properties.find(p.first);
    if (it == properties.end())
      throw runtime_error("Element "+control_id+" has no property "+p.first);
    it->second.name = p.second;
  }

  // If no targets and not optional, create a default one
  if (targets.empty() && !targets_are_optional)
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
          Property(prop, properties.begin()->second.type);
    }

    // Check prefixed attributes
    const auto& props = te.get_attrs_with_prefix("property-");
    for(const auto& p: props)
    {
      const auto& it = properties.find(p.first);
      if (it == properties.end())
        throw runtime_error("Element "+control_id+" has no property "+p.first);
      target.properties[p.first] = Property(p.second, it->second.type);
    }
  }
}

// Attach to a target element
void ControlImpl::attach_target(const string& id, Element *element)
{
  // If we just have the one default one, use this
  auto it = targets.find(id);
  if (it != targets.end())
    it->second.element = element;
}

// Send a value to the target using only (first) property
void ControlImpl::send(const Element::SetParams& sp)
{
  for(const auto& it: targets)
  {
    const Target& target = it.second;
    if (target.element && target.properties.size())
    {
      const auto& p = target.properties.begin();
      target.element->set_property(p->second.name, sp);
    }
  }
}

// Send a named value to the target
// name is our name for it
void ControlImpl::send(const string& name, const Element::SetParams& sp)
{
  for(const auto& it: targets)
  {
    const Target& target = it.second;
    if (target.element)
    {
      const auto& p = target.properties.find(name);
      if (p != target.properties.end())
        target.element->set_property(p->second.name, sp);  // Use their name
      else
      {
        // Look for wildcard, we can send using our name
        const auto& q = target.properties.find("");
        if (q != target.properties.end())
          target.element->set_property(name, sp);  // Use our name
      }
    }
  }
}

}} // namespaces
