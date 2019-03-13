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
    map<string, double> values;  // Property values

    Point(double _at): at(_at) {}
  };

  vector<Point> points;

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
// </interpolate>
// Note both 'at's are optional and assumed 0,1 in this simple case
InterpolateControl::InterpolateControl(const Module *module,
                                       const XML::Element& config):
  Control(module, config)
{
  const auto& point_es = config.get_children("point");
  const auto np = point_es.size();
  double default_at = 0.0;
  for(const auto& it: point_es)
  {
    const XML::Element& te = *it;
    double at = te.get_attr_real("at", default_at);
    if (np > 1) default_at += 1.0 / (np-1);

    Point p(at);

    // Add other attributes as values
    for(const auto ait: te.attrs)
    {
      if (ait.first != "at")
        p.values[ait.first] = Text::stof(ait.second);
    }

    points.push_back(p);
  }
}

//--------------------------------------------------------------------------
// Set a control property
void InterpolateControl::set_property(const string& /*property*/,
                                      const SetParams& sp)
{
  if (!points.size()) return;
  double t = sp.v.d;

  // Values to send
  map<string, double> values;
  double start_at = 0;

  // Find which point we start from
  for(const auto& p: points)
  {
    if (t >= p.at)
    {
      // Start here
      start_at = p.at;
      values = p.values;
    }
    else if (!values.empty())
    {
      // Caught start last time, this point is now the end
      t -= start_at;
      for(auto& it: values)
      {
        // Find value in this point
        const auto it2 = p.values.find(it.first);
        if (it2 != p.values.end())
        {
          double this_span = p.at - start_at;
          if (!this_span) continue;
          double this_t = t / this_span;  // Range to this span
          it.second += this_t * (it2->second - it.second);  // Linear
        }

        // Note if not found it just leaves the previous specified value
      }

      break;
    }
  }

  // Send all values
  for(const auto it: values)
    send(it.first, Value(it.second));
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
    { "t",  { "Proportion to interpolate (0..1)", Value::Type::number, true } }
  },
  { { "", { "Any value", "", Value::Type::any }}} // Flexible controlled property
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(InterpolateControl, module)
