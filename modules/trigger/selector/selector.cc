//==========================================================================
// ViGraph dataflow module: trigger/selector/selector.cc
//
// Selector module - multiple trigger inputs select a single value
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../trigger-module.h"

namespace {

//==========================================================================
// Selector filter
class Selector: public DynamicElement
{
private:
  vector<shared_ptr<Input<Trigger>>> input_list;
  Integer last_inputs{0};
  int current{0};

  // Element virtuals
  void tick(const TickData& td) override;
  void setup(const SetupContext& context) override;

  // Clone
  Selector *create_clone() const override
  {
    return new Selector{module};
  }

public:
  using DynamicElement::DynamicElement;

  // Settings
  Setting<Integer> inputs;

  // Configuration
  Output<Number> output;
};

//--------------------------------------------------------------------------
// Setup inputs
void Selector::setup(const SetupContext& context)
{
  DynamicElement::setup(context);
  if (inputs == last_inputs) return;

  const auto ninputs = static_cast<size_t>(inputs.get());
  while (input_list.size() > ninputs)
  {
    const auto i = input_list.size();
    deregister_input("select" + Text::itos(i), input_list.back().get());
    input_list.pop_back();
  }

  while (input_list.size() < ninputs)
  {
    input_list.emplace_back(new Input<Trigger>{});
    const auto i = input_list.size();
    register_input("select" + Text::itos(i), input_list.back().get());
  }

  last_inputs = inputs;
}

//--------------------------------------------------------------------------
// Tick data
void Selector::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  auto _output{output.get_buffer(td)};

  vector<const vector<Trigger> *> input_bufs;
  for(const auto& input: input_list)
    input_bufs.push_back(&input->get_buffer());

  for(auto i=0u; i<nsamples; i++)
  {
    auto n=1;
    for(const auto input_buf: input_bufs)
    {
      if (i<input_buf->size() && (*input_buf)[i])
        current = n;
      n++;
    }

    _output.data.push_back(current);
  }
}

Dataflow::DynamicModule module
{
  "selector",
  "Selector",
  "trigger",
  {
    { "inputs", &Selector::inputs }
  },
  {},
  {
    { "output", &Selector::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Selector, module)
