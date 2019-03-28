//==========================================================================
// ViGraph dataflow module: vector/filters/translate/translate.cc
//
// Translate filter
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"
#include <cmath>

namespace {

//==========================================================================
// Translate filter
class TranslateFilter: public FrameFilter
{
  Vector start_d, d;

  // Filter/Element virtuals
  void setup() override { start_d = d; }
  void accept(FramePtr frame) override;
  void enable() override;

public:
  using FrameFilter::FrameFilter;

  // Getters/Setters
  double get_x() { return d.x; }
  void set_x(double x) { d.x = x; }
  double get_y() { return d.y; }
  void set_y(double y) { d.y = y; }
  double get_z() { return d.z; }
  void set_z(double z) { d.z = z; }
};

//--------------------------------------------------------------------------
// Enable (reset)
void TranslateFilter::enable()
{
  d = start_d;
}

//--------------------------------------------------------------------------
// Process some data
void TranslateFilter::accept(FramePtr frame)
{
  // Modify all points in the frame
  for(auto& p: frame->points) p+=d;
  send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "translate",
  "Translate",
  "3D translation",
  "vector",
  {
    { "x", { "Distance to move along X axis", Value::Type::number,
         { static_cast<double (Element::*)()>(&TranslateFilter::get_x),
           static_cast<void (Element::*)(double)>(&TranslateFilter::set_x) },
             true } },
    { "y", { "Distance to move along Y axis", Value::Type::number,
         { static_cast<double (Element::*)()>(&TranslateFilter::get_y),
           static_cast<void (Element::*)(double)>(&TranslateFilter::set_y) },
             true } },
    { "z", { "Distance to move along Z axis", Value::Type::number,
         { static_cast<double (Element::*)()>(&TranslateFilter::get_z),
           static_cast<void (Element::*)(double)>(&TranslateFilter::set_z) },
             true } },
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(TranslateFilter, module)
