//==========================================================================
// ViGraph dataflow module: vector/add/add.cc
//
// Add filter - point-by-point addition
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include <cmath>

namespace {

//==========================================================================
// Add
class Add: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Add *create_clone() const override
  {
    return new Add{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Inputs
  Input<Frame> input;
  Input<Frame> offset;

  // Output
  Output<Frame> output;
};

//--------------------------------------------------------------------------
// Tick data
void Add::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(input, offset), tie(output),
                 [&](const Frame& input, const Frame& offset,
                     Frame& output)
  {
    auto isize = input.points.size();
    auto osize = offset.points.size();
    auto common = min(isize, osize);
    auto i=0u;
    for(; i<common; i++)
      output.points.push_back(Point(input.points[i] + offset.points[i],
                                    input.points[i].c));
    for(; i<isize; i++)
      output.points.push_back(input.points[i]);
    for(; i<osize; i++)
      output.points.push_back(offset.points[i]);
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "add",
  "Add",
  "vector",
  {},
  {
    { "input",   &Add::input },
    { "offset",  &Add::offset }
  },
  {
    { "output", &Add::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Add, module)
