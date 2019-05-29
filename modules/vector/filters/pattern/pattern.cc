//==========================================================================
// ViGraph dataflow module: vector/filters/pattern/pattern.cc
//
// Filter to add complex colour patterns
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"

namespace {

//==========================================================================
// Pattern filter
class PatternFilter: public FrameFilter
{
private:
  vector<Colour::RGB> colours;
  enum class BlendType
  {
    none,
    linear
  };
  BlendType blend_type{BlendType::none};

  // Filter/Element virtuals
  void accept(FramePtr frame) override;

public:
  double phase{0};
  int repeats{1};

  // Construct
  PatternFilter(const Dataflow::Module *module, const XML::Element& config);

  // Getters/Setters
  JSON::Value get_colours() const;
  void set_colours(const JSON::Value& json);
  string get_blend() const
  { return (blend_type == BlendType::linear ? "linear" : "none"); }
  void set_blend(const string& b)
  { blend_type = (b == "linear") ? BlendType::linear : BlendType::none; }
};

//--------------------------------------------------------------------------
// Construct from XML
//  <pattern phase="0" repeats="5" blend="linear">
//    <colour>red</colour>
//    <colour>black</colour>
//  </pattern>
PatternFilter::PatternFilter(const Dataflow::Module *module,
                             const XML::Element& config):
  FrameFilter(module, config)
{
  for(const auto& it: config.get_children("colour"))
  {
    const XML::Element& ce = *it;
    Colour::RGB c;
    if (!(*ce).empty())
      c = Colour::RGB(*ce);
    else if (ce.has_attr("h")) // assume HSL
      c = Colour::RGB(Colour::HSL(ce.get_attr_real("h"),
                                  ce.get_attr_real("s", 1.0),
                                  ce.get_attr_real("l", 0.5)));
    else
      c = Colour::RGB(ce.get_attr_real("r"),
                      ce.get_attr_real("g"),
                      ce.get_attr_real("b"));
    colours.push_back(c);
  }
}

//--------------------------------------------------------------------------
// Get colours as JSON value
JSON::Value PatternFilter::get_colours() const
{
  JSON::Value json(JSON::Value::ARRAY);
  for(const auto& it: colours)
    json.add(it.str());

  return json;
}

//--------------------------------------------------------------------------
// Set colours from JSON value
void PatternFilter::set_colours(const JSON::Value& json)
{
  if (json.type != JSON::Value::ARRAY) return;
  colours.clear();
  for(const auto& o: json.a)
  {
    if (o.type != JSON::Value::STRING) continue;
    colours.push_back(Colour::RGB(o.as_str()));
  }
}

//--------------------------------------------------------------------------
// Process some data
void PatternFilter::accept(FramePtr frame)
{
  auto nc = colours.size();
  if (nc)
  {
    // Count non-blanked points
    auto np = 0u;
    for(auto& p: frame->points)
      if (p.is_lit()) np++;

    if (np)
    {
      double i=0.0;
      for(auto& p: frame->points)
      {
        if (p.is_blanked()) continue;

        // Interpolate the colour map into the points, looping
        double frac = i/np + phase;
        double theta = frac*repeats;
        theta -= floor(theta);

        // Theta can become 1.0 due to rounding in the above
        if (theta >= 1.0) theta = 1.0 - 1e-10;
        auto cindex = (unsigned int)floor(theta*nc);
        if (cindex >= nc)
        {
          p.c = Colour::black;  // Double safety
          continue;
        }

        switch (blend_type)
        {
          case BlendType::none:
            p.c = colours[cindex];
            break;

          case BlendType::linear:
          {
            auto next_cindex = (cindex+1)%nc;
            auto blend = theta*nc - cindex;
            p.c = colours[cindex].blend_with(colours[next_cindex], blend);
            break;
          }
        }

        i++;
      }
    }
  }

  send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "pattern",
  "Pattern",
  "Adds a pattern of colours, optionally blending",
  "vector",
  {
    { "colours", { "Sequence of colours", "colours",
                 { &PatternFilter::get_colours,
                   &PatternFilter::set_colours }, true } },
    { "phase", { "Phase of pattern (0..1)", Value::Type::number,
                 &PatternFilter::phase, true } },
    { "repeats", { "Number of repeats of pattern",
                   Value::Type::number, &PatternFilter::repeats, true } },
    { "blend", { "Type of blending to use", Value::Type::choice,
                 { &PatternFilter::get_blend, &PatternFilter::set_blend },
                 { "none", "linear" }, true } }
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(PatternFilter, module)
