//==========================================================================
// ViGraph dataflow module: object/get-number/get-number.cc
//
// Extract a number property from an object
//
// Copyright (c) 2021 Paul Clark.  All rights reserved
//==========================================================================

#include "../object-module.h"

namespace {

class GetNumber: public SimpleElement
{
private:
  Number last_value{0};

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  GetNumber *create_clone() const override
  {
    return new GetNumber{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Setting<string> property;

  // Input
  Input<Data> input;

  // Output
  Output<Number> output;
};

//--------------------------------------------------------------------------
// Tick data
void GetNumber::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, tie(property), tie(input), tie(output),
                 [&](const string& property, const Data& input, Number& output)
  {
    if (input.json.type == JSON::Value::OBJECT)
      output = last_value = input.json[property].as_float(last_value);
    else
      output = last_value;
  });
}

Dataflow::SimpleModule module
{
  "get-number",
  "Get number",
  "object",
  {
    { "property", &GetNumber::property }
  },
  {
    { "input",    &GetNumber::input    }
  },
  {
    { "output", &GetNumber::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(GetNumber, module)
