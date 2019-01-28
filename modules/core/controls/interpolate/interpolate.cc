//==========================================================================
// ViGraph dataflow module: controls/interpolate/interpolate.cc
//
// Control to alter one or more properties through interpolation
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>

namespace {

//==========================================================================
// Interpolate control
class InterpolateControl: public Control
{
  // Fixed points
  struct Point
  {
    double at;
    map<string, Value> values;  // Property values

    Point(double _at): at(_at) {}
  };

  list<Point> points;

  // Control virtuals
  void set_property(const string& property, const SetParams& sp) override;

public:
  // Construct
  InterpolateControl(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
// <interpolate property="...">
//   <point at="0" x="0"/>
//   <point at="1" x="0.5"/>
// <interpolate>
// Note both 'at's are optional and assumed 0,1 in this simple case
InterpolateControl::InterpolateControl(const Module *module,
                                       const XML::Element& config):
  Element(module, config), Control(module, config)
{
  // !!! Read points
}

//--------------------------------------------------------------------------
// Set a control property
void InterpolateControl::set_property(const string& /*property*/,
                                      const SetParams& sp)
{
  // Interpolate using this value !!!
  send(sp);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "interpolate",
  "Interpolate",
  "Animate one or more properties using key frame interpolation",
  "core",
  {
    { "",  { "Proportion to interpolate (0..1)", Value::Type::number, true } }
  },
  { { "", { "Any value", "", Value::Type::any }}} // Flexible controlled property
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(InterpolateControl, module)
