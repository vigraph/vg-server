//==========================================================================
// ViGraph dataflow module: vector/filters/vector-to-rgb/vector-to-rgb.cc
//
// Vector to RGB filter - extracts colour from first lit point, allowing
// use of vector colour operations with (e.g.) DMX
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"

namespace {

//==========================================================================
// VectorToRGB filter
class VectorToRGBFilter: public FrameFilter, public Dataflow::ControlImpl
{
private:
  // Filter/Element virtuals
  void accept(FramePtr frame) override;

  // Add control JSON
  JSON::Value get_json(const string& path) const override
  { JSON::Value json=FrameFilter::get_json(path);
    add_to_json(json); return json; }

public:
  // Construct
  VectorToRGBFilter(const Dataflow::Module *module,
                        const XML::Element& config):
    FrameFilter(module, config),
    ControlImpl(module, config, true)
  {}
};

//--------------------------------------------------------------------------
// Process some data
void VectorToRGBFilter::accept(FramePtr frame)
{
  for(const auto& p: frame->points)
  {
    if (p.is_lit())
    {
      ControlImpl::send("r", p.c.r);
      ControlImpl::send("g", p.c.g);
      ControlImpl::send("b", p.c.b);
      return;
    }
  }

  // No lit points
  ControlImpl::send("r", 0.0);
  ControlImpl::send("g", 0.0);
  ControlImpl::send("b", 0.0);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "vector-to-rgb",
  "Vector to RGB",
  "Extracts colour of first point of a vector as separate R,G,B values",
  "vector",
  {  },
  {
    { "r", { "Red output",   "r", Value::Type::number }},
    { "g", { "Green output", "g", Value::Type::number }},
    { "b", { "Blue output",  "b", Value::Type::number }}
  },
  { "VectorFrame" }, // inputs
  { } // no outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(VectorToRGBFilter, module)
