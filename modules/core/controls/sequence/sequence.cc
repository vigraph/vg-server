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
  unsigned index = 0;

public:
  bool loop = true;

  // Construct
  SequenceControl(const Module *module, const XML::Element& config);

  // Getters/Setters
  int get_index() const { return index; }
  void set_index(int index);
  void next();
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
  values = Text::split_words(config["values"]);
  if (values.empty())
  {
    const auto els = config.get_children("value");
    for (const auto& el: els)
      values.push_back(el->content);
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
// Module definition
Dataflow::Module module
{
  "sequence",
  "Sequence",
  "Sequence a set of control values",
  "core",
  {
    { "index", { "Index number", Value::Type::number,
                 { &SequenceControl::get_index, &SequenceControl::set_index },
                 true } },
    { "loop", { "Loop at end of sequence", Value::Type::boolean,
                &SequenceControl::loop, true } },
    { "next", { "Trigger move to next entry", Value::Type::trigger,
                &SequenceControl::next, true } },
  },
  { { "value", { "Value output", "value", Value::Type::text }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SequenceControl, module)
