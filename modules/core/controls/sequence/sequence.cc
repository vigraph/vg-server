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
  vector<string> values;
  unsigned index = 0;

public:
  bool loop = true;

  // Construct
  using Control::Control;

  // Getters/Setters
  JSON::Value get_values() const;
  void set_values(const JSON::Value& json);
  int get_index() const { return index; }
  void set_index(int index);
  void next();
  void prev();
};

//--------------------------------------------------------------------------
// Get values as JSON value
JSON::Value SequenceControl::get_values() const
{
  JSON::Value json(JSON::Value::ARRAY);
  for(const auto& it: values)
    json.add(it);

  return json;
}

//--------------------------------------------------------------------------
// Set values from JSON value
void SequenceControl::set_values(const JSON::Value& json)
{
  if (json.type != JSON::Value::ARRAY) return;
  values.clear();
  for(const auto& o: json.a)
  {
    if (o.type != JSON::Value::STRING) continue;
    values.push_back(o.as_str());
  }
}

//--------------------------------------------------------------------------
// Set index
void SequenceControl::set_index(int i)
{
  index = i;
  if (index >= values.size())
    index = values.size() - 1;
  send({values[index]});
}

//--------------------------------------------------------------------------
// Move to next entry
void SequenceControl::next()
{
  if (++index >= values.size())
  {
    if (loop)
      index = 0;
    else
      index = values.size() - 1;
  }

  send({values[index]});
}

//--------------------------------------------------------------------------
// Move to previous entry
void SequenceControl::prev()
{
  if (--index >= values.size())
  {
    if (loop)
      index = values.size() - 1;
    else
      index = 0;
  }

  send({values[index]});
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
    { "values", { "Sequence values", "sequence",
                 { &SequenceControl::get_values,
                   &SequenceControl::set_values }, true } },
    { "index", { "Index number", Value::Type::number,
                 { &SequenceControl::get_index, &SequenceControl::set_index },
                 true } },
    { "loop", { "Loop at end of sequence", Value::Type::boolean,
                &SequenceControl::loop, true } },
    { "next", { "Trigger move to next entry", Value::Type::trigger,
                &SequenceControl::next, true } },
    { "prev", { "Trigger move to previous entry", Value::Type::trigger,
                &SequenceControl::prev, true } },
  },
  { { "output", { "Value output", "value", Value::Type::text }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SequenceControl, module)
