//==========================================================================
// ViGraph dataflow module: core/function/function.cc
//
// Function module - multiple trigger inputs select a single value
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../core-module.h"
#include <cmath>
#include <cfloat>
#include "func.h"

namespace {

//==========================================================================
// Function filter
class Function: public DynamicElement
{
private:
  Func last_func{Func::none};

  // Element virtuals
  void tick(const TickData& td) override;
  void setup(const SetupContext& context) override;

  // Clone
  Function *create_clone() const override
  {
    return new Function{module};
  }

public:
  using DynamicElement::DynamicElement;

  // Settings
  Setting<Func> func;

  // Dynamically useable inputs
  Input<Number> input;
  Input<Number> x;
  Input<Number> y;

  // Configuration
  Output<Number> output;
};

//--------------------------------------------------------------------------
// Setup inputs
void Function::setup(const SetupContext&)
{
  int ninputs;
  switch (func)
  {
    case Func::none:
      ninputs = 0;
      break;

    case Func::mod:
    case Func::power:
      ninputs = 2;
      break;

    default:
      ninputs = 1;
      break;
  }

  int last_ninputs;
  switch (last_func)
  {
    case Func::none:
      last_ninputs = 0;
      break;

    case Func::mod:
    case Func::power:
      last_ninputs = 2;
      break;

    default:
      last_ninputs = 1;
      break;
  }

  if (ninputs == last_ninputs) return;

  // Remove old
  switch (last_ninputs)
  {
    case 1:
      module.erase_input("input");
      break;

    case 2:
      module.erase_input("x");
      module.erase_input("y");
      break;
  }

  // Add new
  switch (ninputs)
  {
    case 1:
      module.add_input("input", &Function::input);
      break;

    case 2:
      module.add_input("x", &Function::x);
      module.add_input("y", &Function::y);
      break;
  }

  last_func = func;
}

//--------------------------------------------------------------------------
// Tick data
void Function::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {},
                 tie(input, x, y), tie(output),
                   [&](Number input, Number x, Number y,
                       Number& output)
  {
    switch (func)
    {
      case Func::none:                         break;
      case Func::sin:   output = sin(input);   break;
      case Func::cos:   output = cos(input);   break;
      case Func::tan:   output = tan(input);   break;
      case Func::asin:  output = asin(input);  break;
      case Func::acos:  output = acos(input);  break;
      case Func::atan:  output = atan(input);  break;
      case Func::log10: output = log10(input); break;
      case Func::log:   output = log(input);   break;
      case Func::exp10: output = exp10(input); break;
      case Func::exp:   output = exp(input);   break;
      case Func::sqrt:  output = sqrt(input);  break;
      case Func::square: output = input*input;       break;
      case Func::cube:   output = input*input*input; break;
      case Func::inverse: output = input ? (1.0/input) : DBL_MAX; break;
      case Func::floor: output = floor(input); break;
      case Func::ceil:  output = ceil(input);  break;
      case Func::round: output = round(input); break;
      case Func::abs:   output = abs(input);   break;

      case Func::mod:   output = y ? fmod(x, y): 0;   break;
      case Func::power: output = pow(x, y);    break;
    }
  });
}

Dataflow::DynamicModule module
{
  "function",
  "Function",
  "core",
  {
    { "f", &Function::func }

    // + Dynamically set 'input' or { 'x', 'y' }
  },
  {},
  {
    { "output", &Function::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Function, module)
