//==========================================================================
// ViGraph dataflow module: vector/slice/slice.cc
//
// Slice filter
//
// Copyright (c) 2029 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include <cmath>

namespace {

//==========================================================================
// Slice
class Slice: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Slice *create_clone() const override
  {
    return new Slice{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<Number> start{0.0};
  Input<Number> length{1.0};

  // Input
  Input<Frame> input;

  // Output
  Output<Frame> output;
};

//--------------------------------------------------------------------------
// Tick data
void Slice::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(start, length, input), tie(output),
                 [&](Number start, Number length, const Frame& input,
                     Frame& output)
  {
    auto size = input.points.size();
    start = min(max(start, 0.0), 1.0);
    length = min(max(length, 0.0), 1.0);
    auto start_point = min(static_cast<size_t>(start * size), size);
    auto n = static_cast<size_t>(length * size);
    for(auto i=0u; i<n; i++)
    {
      if (i)
      {
        auto j = start_point + i;
        if (j >= size) j-=size;  // wrap
        output.points.push_back(input.points[j]);
      }
      else
      {
        // first is always blanked
        output.points.push_back(Vector(input.points[start_point]));
      }
    }
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "slice",
  "Slice",
  "vector",
  {},
  {
    { "start",  &Slice::start  },
    { "length", &Slice::length },
    { "input",  &Slice::input }
  },
  {
    { "output", &Slice::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Slice, module)
