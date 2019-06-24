//==========================================================================
// ViGraph dataflow module: controls/trigger-sequence/trigger-sequence.cc
//
// Control to send triggers based on an sequence
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>

namespace {

//==========================================================================
// Trigger Sequence control
class TriggerSequenceControl: public Dataflow::Control
{
  vector<bool> values;
  unsigned index = 0;

  // Send output for given index
  void do_send(unsigned index);

public:
  bool loop = true;
  int entry_index = 0;

  // Construct
  TriggerSequenceControl(const Module *module, const XML::Element& config);

  // Getters/Setters
  JSON::Value get_values() const;
  void set_values(const JSON::Value& json);
  int get_index() const { return index; }
  void set_index(int index);
  void set_entry_value(bool value);
  void next();
};

//--------------------------------------------------------------------------
// Construct from XML
//   <trigger-sequence property="x">
//     <value>on</value>
//     <value>off</value>
//   </trigger-sequence>
TriggerSequenceControl::TriggerSequenceControl(const Module *module,
                                               const XML::Element& config):
  Control(module, config)
{
  const auto vl = Text::split_words(config["value-list"]);
  for (const auto& v: vl)
    values.push_back(Text::stob(v));
  if (values.empty())
  {
    const auto els = config.get_children("value");
    for (const auto& el: els)
      values.push_back(Text::stob(el->content));
  }
}

//--------------------------------------------------------------------------
// Get values as JSON value
JSON::Value TriggerSequenceControl::get_values() const
{
  JSON::Value json(JSON::Value::ARRAY);
  for(const auto& it: values)
  {
    if (it)
      json.add(JSON::Value::TRUE_);
    else
      json.add(JSON::Value::FALSE_);
  }

  return json;
}

//--------------------------------------------------------------------------
// Set values from JSON value
void TriggerSequenceControl::set_values(const JSON::Value& json)
{
  if (json.type != JSON::Value::ARRAY) return;
  values.clear();
  for(const auto& o: json.a)
  {
    if (o.type == JSON::Value::TRUE_)
      values.push_back(true);
    else if (o.type == JSON::Value::FALSE_)
      values.push_back(false);
  }
}

//--------------------------------------------------------------------------
// Send output for given index
void TriggerSequenceControl::do_send(unsigned i)
{
  if (i < values.size())
  {
    if (values[i])
      trigger("output");
    send("position", i);
    trigger("position-trigger");
  }
}

//--------------------------------------------------------------------------
// Set index
void TriggerSequenceControl::set_index(int i)
{
  index = i;
  if (index >= values.size())
    index = values.size() - 1;
  do_send(index);
}

//--------------------------------------------------------------------------
// Set value of an entry
void TriggerSequenceControl::set_entry_value(bool value)
{
  if (entry_index >= 0 && static_cast<unsigned>(entry_index) < values.size())
    values[entry_index] = value;
}

//--------------------------------------------------------------------------
// Move to next entry
void TriggerSequenceControl::next()
{
  if (++index >= values.size())
  {
    if (loop)
      index = 0;
    else
      index = values.size() - 1;
  }
  do_send(index);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "trigger-sequence",
  "TriggerSequence",
  "Sequence a set of triggers",
  "core",
  {
    { "values", { "Sequence values", "sequence",
                 { &TriggerSequenceControl::get_values,
                   &TriggerSequenceControl::set_values }, true } },
    { "index", { "Index number", Value::Type::number,
                 { &TriggerSequenceControl::get_index,
                   &TriggerSequenceControl::set_index },
                 true } },
    { "entry", { "Entry index", Value::Type::number,
                 &TriggerSequenceControl::entry_index,
                 true } },
    { "entry-value", { "Entry index", Value::Type::boolean,
                       { &TriggerSequenceControl::set_entry_value },
                       true } },
    { "loop", { "Loop at end of sequence", Value::Type::boolean,
                &TriggerSequenceControl::loop, true } },
    { "next", { "Trigger move to next entry", Value::Type::trigger,
                &TriggerSequenceControl::next, true } },
  },
  {
    { "output", { "Trigger output", "trigger", Value::Type::trigger }},
    { "position", { "Current position in sequence", "position",
                    Value::Type::number }},
    { "position-trigger", { "Triggered when position changes",
                            "position-trigger", Value::Type::trigger }},
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(TriggerSequenceControl, module)
