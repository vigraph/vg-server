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
//   <point at="0" x="0"/>
//   <point at="1" x="0.5"/>
// </interpolate>
// Note both 'at's are optional and assumed 0,1 in this simple case
// !!! No way to replace this at the moment
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
// Get curve as JSON value
JSON::Value InterpolateControl::get_curve() const
{
  JSON::Value json(JSON::Value::ARRAY);
  for(const auto& p: points)
  {
    JSON::Value o(JSON::Value::OBJECT);
    o.set("at", p.at);
    for(const auto& pit: p.values)
      o.set(pit.first, pit.second);
  }

  return json;
}

//--------------------------------------------------------------------------
// Set curve from JSON value
void InterpolateControl::set_curve(const JSON::Value& json)
{
  if (json.type != JSON::Value::ARRAY) return;
  points.clear();
  double default_at = 0.0;
  const auto np = json.a.size();
  for(const auto& o: json.a)
  {
    if (o.type != JSON::Value::OBJECT) continue;
    double at = o["at"].as_float(default_at);
    if (np > 1) default_at += 1.0 / (np-1);

    Point p(at);

    // Add other attributes as values
    for(const auto& pit: o.o)
    {
      if (pit.first != "at")
        p.values[pit.first] = pit.second.as_float();
    }

    points.push_back(p);
  }
}

//--------------------------------------------------------------------------
// Set 't'
void InterpolateControl::set_t(double t)
{
  if (!points.size()) return;

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
    { "t",  { "Proportion to interpolate (0..1)", Value::Type::number,
              { &InterpolateControl::get_t, &InterpolateControl::set_t },
              true }},
    { "curve", { "Interpolation curve", "curve",
                 { &InterpolateControl::get_curve,
                   &InterpolateControl::set_curve }, } }
  },
  { { "", { "Any value", "", Value::Type::any }}} // Flexible controlled property
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(InterpolateControl, module)
