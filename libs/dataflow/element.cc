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
// Get a property value as JSON
JSON::Value Element::get_property_json(const Module::Property& prop) const
{
  // Ignore aliases
  if (prop.alias) return {};

  const auto& member = prop.member;

  // Get value from prop through member pointers or getter functions
  if (member.d_ptr)
    return JSON::Value(JSON::Value::NUMBER, this->*member.d_ptr);
  else if (member.s_ptr)
    return JSON::Value(this->*member.s_ptr);
  else if (member.b_ptr)
    return JSON::Value((this->*member.b_ptr)?JSON::Value::TRUE_
                       :JSON::Value::FALSE_);
  else if (member.i_ptr)
    return JSON::Value(this->*member.i_ptr);
  else if (member.get_d)
    return JSON::Value(JSON::Value::NUMBER, (this->*member.get_d)());
  else if (member.get_s)
    return JSON::Value((this->*member.get_s)());
  else if (member.get_b)
    return JSON::Value((this->*member.get_b)()?JSON::Value::TRUE_
                       :JSON::Value::FALSE_);
  else if (member.get_i)
    return JSON::Value((this->*member.get_i)());
  else if (member.get_json)
    return (this->*member.get_json)();
  else
    return {};
}

//------------------------------------------------------------------------
// Set an output from JSON
void Element::set_output_json(const string& path, const JSON::Value& value)
{
  if (path == "default") // !!! For now, later look up in named outputs
  {
    // Only Generators
    const auto this_g = dynamic_cast<Generator *>(this);
    if (this_g)
    {
      this_g->set_output_from_json(path, value);
      graph->generate_topological_order(); // !!! Uses downstreams
    }
    else
      throw runtime_error("Element "+id+" has no data outputs");
  }
  else
  {
    // Look for controlled property target to redirect
    const auto cpit = module->controlled_properties.find(path);
    if (cpit != module->controlled_properties.end())
    {
      // Only for controls
      ControlImpl *this_c = dynamic_cast<ControlImpl *>(this);
      if (this_c)
      {
        this_c->set_target_from_json(path, value, this);
        graph->generate_topological_order(); // !!! Uses downstreams
      }
    }
    else
      throw runtime_error("No such property "+path+" in element "+id);
  }
}

//------------------------------------------------------------------------
// Set state from JSON
void Element::set_json(const string& path, const JSON::Value& value)
{
  if (path.empty())
  {
    // Whole element
    if (value.type != JSON::Value::OBJECT)
      throw runtime_error("Setting a whole element must use a JSON object");

    // Look for properties
    const auto& props = value["props"];
    if (props.type == JSON::Value::OBJECT)
    {
      // Do each property individually
      for(const auto& it: props.o)
      {
        const auto pit = module->properties.find(it.first);
        // Make this forward compatible
        if (pit == module->properties.end()) continue;
        set_property(it.first, pit->second, Value(it.second));
      }
    }

    // And outputs
    const auto& outputs = value["outputs"];
    if (outputs.type == JSON::Value::OBJECT)
    {
      // Do each output individually
      for(const auto& it: outputs.o)
        set_output_json(it.first, it.second);
    }

    // Note, can't set 'id' or 'type', these have to be set by an add to
    // level above - we just ignore them
  }
  else
  {
    // Individual property - note must be leaf path now
    const auto pit = module->properties.find(path);
    if (pit != module->properties.end())
    {
      set_property(path, pit->second, Value(value));
    }
    else set_output_json(path, value);
  }

  // Action changes
  update();
}

//------------------------------------------------------------------------
// Get state as JSON
JSON::Value Element::get_json(const string& path) const
{
  if (path.empty())
  {
    // Whole element
    JSON::Value json(JSON::Value::Type::OBJECT);
    json.set("id", id);
    if (module) json.set("type", module->section+":"+module->id);

    JSON::Value& propsj = json.set("props", JSON::Value(JSON::Value::OBJECT));
    for(const auto pit: module->properties)
    {
      JSON::Value value = get_property_json(pit.second);
      // Invalid ones can just be ignored
      if (!!value) propsj.set(pit.first, value);
    }
    return json;
  }
  else
  {
    // Individual property - note must be leaf path now
    const auto pit = module->properties.find(path);
    if (pit == module->properties.end())
      throw runtime_error("No such property "+path+" in element "+id);
    JSON::Value value = get_property_json(pit->second);
    if (!value)
      throw runtime_error("No method to get property "+path
                          +" from element "+id);
    return value;
  }
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
void Element::configure(const File::Directory& base_dir,
                        const XML::Element& config)
{
  // Check all properties to see if attribute exists
  for(const auto pit: module->properties)
  {
    const auto& name = pit.first;
    if (name.empty())
    {
      // Value is the element content
      if (!config.content.empty())
        configure_property(name, pit.second, config.content);
    }
    else
    {
      // Can be either direct attribute ('points') or sub-element ('x.freq')
      vector<string> bits = Text::split(name, '.');
      string value;

      switch (bits.size())
      {
        case 1:
          if (!config.has_attr(bits[0])) continue;
          value = config[bits[0]];
          break;

        case 2:
        {
          // Look down a level
          const auto& sub = config.get_child(bits[0]);
          if (!sub.has_attr(bits[1])) continue;
          value = sub[bits[1]];
          break;
        }

        default: throw runtime_error("Weird property name "+name);
      }

      configure_property(name, pit.second, value);
    }
  }

  setup(base_dir);
}

//------------------------------------------------------------------------
// Configure with an property name and string value
void Element::configure_property(const string& name,
                                 const Module::Property& prop,
                                 const string& value)
{
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
void Element::set_property(const string& prop_name,
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
      else if (member.set_multi_d)
        (this->*member.set_multi_d)({v.d});
      else
        throw runtime_error("No member pointers for numeric property "+prop_name
                            +" in element "+id);
      break;

    case Value::Type::text:
    case Value::Type::file:
    case Value::Type::choice:
      if (member.s_ptr)
        this->*member.s_ptr = v.s;
      else if (member.set_s)
        (this->*member.set_s)(v.s);
      else
        throw runtime_error("No member pointers for string property "+prop_name
                            +" in element "+id);
      break;

    case Value::Type::trigger:
      if (member.trigger)
        (this->*member.trigger)();
      else
        throw runtime_error("No function for trigger property "+prop_name
                            +" in element "+id);
      break;

    default:
      throw runtime_error("Unsettable type in property "+prop_name
                          +" in element "+id);
  }
}

//------------------------------------------------------------------------
// Set a property (internal, with property already looked up)
void Element::set_property(const string& prop_name,
                           const Module::Property& prop,
                           const vector<double>& v)
{
  if (v.empty())
    return;
  const Module::Property::Member& member = prop.member;
  if (member.set_multi_d)
    (this->*member.set_multi_d)(v);
  else if (member.d_ptr)
    this->*member.d_ptr = v.front();
  else if (member.set_d)
    (this->*member.set_d)(v.front());
  else
    throw runtime_error("No member pointers for numeric property "+prop_name
                        +" in element "+id);
}

//------------------------------------------------------------------------
// Set a property (external)
void Element::set_property(const string& prop_name,
                           const Value& v)
{
  try
  {
    const auto pit = module->properties.find(prop_name);
    if (pit == module->properties.end())
      throw runtime_error("No such property "+prop_name+" on element "+id);
    set_property(prop_name, pit->second, v);

    // Action changes
    // !!! Later optimise to only call once for sets of updates?
    update();
  }
  catch (const runtime_error& e)
  {
    Log::Error log;
    log << "Set property failed for " << id << ": " << e.what() << endl;
  }
}

//------------------------------------------------------------------------
// Set a property with multiple values (external)
void Element::set_property(const string& prop_name,
                           const vector<double>& v)
{
  try
  {
    const auto pit = module->properties.find(prop_name);
    if (pit == module->properties.end())
      throw runtime_error("No such property "+prop_name+" on element "+id);
    set_property(prop_name, pit->second, v);

    // Action changes
    // !!! Later optimise to only call once for sets of updates?
    update();
  }
  catch (const runtime_error& e)
  {
    Log::Error log;
    log << "Set property failed for " << id << ": " << e.what() << endl;
  }
}

}} // namespaces
