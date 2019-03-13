//==========================================================================
// ViGraph dataflow module: controls/sequence/sequence.cc
//
// Control to set a text property based on an sequence of values
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>

namespace {

//==========================================================================
// Sequence control
class SequenceControl: public Dataflow::Control
{
  string property;
  vector<string> values;
  bool loop = true;
  unsigned pos = 0;

  // Control/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  Dataflow::Value::Type get_property_type(const string& property) override;

public:
  // Construct
  SequenceControl(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//   <sequence property="x">
//     <value>one</value>
//     <value>two</value>
//   </sequence>
SequenceControl::SequenceControl(const Module *module, const XML::Element& config):
  Control(module, config)
{
  property = config["property"];
  loop = config.get_attr_bool("loop", loop);

  values = Text::split_words(config["values"]);
  if (values.empty())
  {
    const auto els = config.get_children("value");
    for (const auto& el: els)
      values.push_back(el->content);
  }
}

//--------------------------------------------------------------------------
// Set a control property
void SequenceControl::set_property(const string& prop,
                                   const SetParams& sp)
{
  if (prop == "index")
  {
    pos = sp.v.d;
    if (pos >= values.size())
      pos = values.size() - 1;
  }
  else if (prop == "trigger")
  {
    if (++pos >= values.size())
    {
      if (loop)
        pos = 0;
      else
        pos = values.size() - 1;
    }
  }
  else if (prop == "loop")
  {
    update_prop(loop, sp);
    return;
  }
  else return;

  SetParams nsp(values[pos]);
  send(property, nsp);
}

//--------------------------------------------------------------------------
// Get control property types
Dataflow::Value::Type
  SequenceControl::get_property_type(const string& prop)
{
  if (prop == property)
    return Dataflow::Value::Type::text;

  return Dataflow::Value::Type::invalid;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "sequence",
  "Sequence",
  "Sequence a set of control values",
  "core",
  {
    { "property", { "Property to set", Value::Type::text } },
    { "index", { "Index number", Value::Type::number } },
    { "loop", { "Loop at end of sequence", Value::Type::boolean } },
  },
  { { "", { "Value output", "", Value::Type::text }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SequenceControl, module)
