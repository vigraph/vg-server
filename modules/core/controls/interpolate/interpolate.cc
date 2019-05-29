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
  map<double, double> curve;  // t -> value

public:
  // Construct
  InterpolateControl(const Module *module, const XML::Element& config);

  // Getter/setters
  JSON::Value get_curve() const;
  void set_curve(const JSON::Value& json);
  double get_t() const { return 0.0; }
  void set_t(double t);
};

//--------------------------------------------------------------------------
// Construct from XML
// <interpolate property="...">
//   <point t="0" value="0"/>
//   <point t="1" value="0.5"/>
// </interpolate>
// Note both 't's are optional and assumed 0,1 in this simple case
InterpolateControl::InterpolateControl(const Module *module,
                                       const XML::Element& config):
  Control(module, config)
{
  const auto& point_es = config.get_children("point");
  const auto np = point_es.size();
  double default_t = 0.0;
  for(const auto& it: point_es)
  {
    const XML::Element& te = *it;
    double t = te.get_attr_real("t", default_t);
    if (np > 1) default_t += 1.0 / (np-1);

    double value = te.get_attr_real("value");
    curve[t] = value;
  }
}

//--------------------------------------------------------------------------
// Get curve as JSON value
JSON::Value InterpolateControl::get_curve() const
{
  JSON::Value json(JSON::Value::ARRAY);
  for(const auto& it: curve)
  {
    auto& o = json.add(JSON::Value(JSON::Value::OBJECT));
    o.set("t", it.first);
    o.set("value", it.second);
  }

  return json;
}

//--------------------------------------------------------------------------
// Set curve from JSON value
void InterpolateControl::set_curve(const JSON::Value& json)
{
  if (json.type != JSON::Value::ARRAY) return;
  curve.clear();
  double default_t = 0.0;
  const auto np = json.a.size();
  for(const auto& o: json.a)
  {
    if (o.type != JSON::Value::OBJECT) continue;
    double t = o["t"].as_float(default_t);
    if (np > 1) default_t += 1.0 / (np-1);

    double value = o["value"].as_float();
    curve[t] = value;
  }
}

//--------------------------------------------------------------------------
// Set 't'
void InterpolateControl::set_t(double t)
{
  if (!curve.size()) return;

  double start_t{0};
  double value{0};
  bool started{false};

  // Find which point we start from
  for(const auto& it: curve)
  {
    if (t >= it.first)
    {
      // Start here
      start_t = it.first;
      value = it.second;
      started = true;
    }
    else if (started)
    {
      // Caught start last time, this point is now the end
      t -= start_t;

      double this_span = it.first - start_t;
      if (!this_span) continue;
      double this_t = t / this_span;  // Range to this span
      value += this_t * (it.second - value);  // Linear
      break;
    }
  }

  // Send the value
  send(Value(value));
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
    { "t",  { "Proportion to interpolate (0..1)", Value::Type::number,
              { &InterpolateControl::get_t, &InterpolateControl::set_t },
              true }},
    { "curve", { "Interpolation curve", "curve",
                 { &InterpolateControl::get_curve,
                   &InterpolateControl::set_curve }, true } }
  },
  { { "value", { "Value output", "value", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(InterpolateControl, module)
