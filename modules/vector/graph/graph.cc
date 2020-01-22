//==========================================================================
// ViGraph dataflow module: vector/graph/graph.cc
//
// Graph filter
//
// Copyright (c) 2017-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include <cmath>
#include <cfloat>

namespace {

const auto default_sample_rate = 1000;
const auto default_points = 1000;

//==========================================================================
// GraphSink
class GraphSink: public SimpleElement
{
private:
  vector<Number> buffer;
  bool running{false};

  // Element virtuals
  void setup(const SetupContext& context) override;
  void update_sample_rate() override {}
  void tick(const TickData& td) override;

  // Clone
  GraphSink *create_clone() const override
  {
    return new GraphSink{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Setting<Number> sample_rate{default_sample_rate};
  Setting<Number> points{default_points};
  Setting<bool> scroll{false};
  Setting<bool> auto_range{false};

  // Input
  Input<Number> input;
  Input<Trigger> start;
  Input<Trigger> clear;

  // Output
  Output<Frame> output;
};

//--------------------------------------------------------------------------
// Setup
void GraphSink::setup(const SetupContext& context)
{
  SimpleElement::setup(context);
  input.set_sample_rate(sample_rate);
  start.set_sample_rate(sample_rate);
  clear.set_sample_rate(sample_rate);
}

//--------------------------------------------------------------------------
// Tick data
void GraphSink::tick(const TickData& td)
{
  const auto& in = input.get_buffer();

  if (running && points)
  {
    buffer.insert(buffer.end(), in.begin(), in.end());

    if (buffer.size() >= points)
    {
      if (scroll)
      {
        // Prune from front
        buffer.erase(buffer.begin(), buffer.begin()+(buffer.size()-points));
      }
      else
      {
        // Throw away excess and stop
        buffer.erase(buffer.begin()+points, buffer.end());
        running = false;
      }
    }
  }

  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {},
                 tie(start, clear),
                 tie(output),
                 [&](Trigger _start, Trigger clear,
                     Frame& output)
  {
    if (clear)
    {
      buffer.clear();
      running = false;
    }
    else if (_start || !start.connected())
    {
      running = true;
    }

    auto low{0.0};
    auto high{1.0};

    if (auto_range && !buffer.empty())
    {
      low = DBL_MAX;
      high = DBL_MIN;
      for(auto v: buffer)
      {
        if (v < low) low = v;
        if (v > high) high = v;
      }

      if (low == high) high++;
    }

    for(auto i=0u; i<buffer.size(); i++)
    {
      const auto x = static_cast<double>(i) / (points>1 ? points-1 : 1) - 0.5;
      const auto y = (buffer[i]-low)/(high-low) - 0.5;

      // Double first point with extra blank to start
      if (!i) output.points.emplace_back(x, y);
      output.points.emplace_back(x, y, Colour::white);
    }
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "graph",
  "Graph",
  "vector",
  {
    { "sample-rate",  &GraphSink::sample_rate },
    { "scroll",       &GraphSink::scroll },
    { "auto-range",   &GraphSink::auto_range },
    { "points",       &GraphSink::points }
  },
  {
    { "input",        &GraphSink::input },
    { "start",        &GraphSink::start },
    { "clear",        &GraphSink::clear }
  },
  {
    { "output",       &GraphSink::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(GraphSink, module)
