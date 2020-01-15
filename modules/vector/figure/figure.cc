//==========================================================================
// ViGraph dataflow module: vector/figure/figure.cc
//
// Generic abstract figure source, usable for lines, circles, waves,
// Lissajous etc.
//
// Inputs:
//    closed: whether to add a final point equal to first
//    points: number of points per frame (default 100)
//   x, y, z axes each have
//    ?-wave: waveform: none, saw, sine, square, triangle (default none)
//    ?-freq: wave iterations per frame (default 1)
//    ?-phase: wave phase offset, degrees/360 (0..1) (default 0)
//    ?-pulse-width: wave pulse width
//
// Point beam:
//   figure points=1
//
// Horizontal line:
//   figure points=10 x-wave="saw"
//
// Circle:
//   figure x-wave="sin" y-wave="sin" y-phase=0.25
//
// Lissajous:
//   figure x-wave="sin" x-freq=5 y-wave="sin" y-freq=3
//
// Copyright (c) 2017-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include "../../waveform.h"
#include <cmath>

namespace {

const auto default_points{100};

//==========================================================================
// Figure
class Figure: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Figure *create_clone() const override
  {
    return new Figure{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Inputs
  Input<Waveform::Type> x_waveform;
  Input<Number> x_freq{1.0};
  Input<Number> x_phase{0.0};
  Input<Number> x_pulse_width{0.5};

  Input<Waveform::Type> y_waveform;
  Input<Number> y_freq{1.0};
  Input<Number> y_phase{0.0};
  Input<Number> y_pulse_width{0.5};

  Input<Waveform::Type> z_waveform;
  Input<Number> z_freq{1.0};
  Input<Number> z_phase{0.0};
  Input<Number> z_pulse_width{0.5};

  Input<Number> points{default_points};
  Input<Number> closed{0.0};

  // Outputs
  Output<Frame> output;
};

//--------------------------------------------------------------------------
// Generate a fragment
void Figure::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {},
                 tie(x_waveform, x_freq, x_phase, x_pulse_width,
                     y_waveform, y_freq, y_phase, y_pulse_width,
                     z_waveform, z_freq, z_phase, z_pulse_width,
                     points, closed),
                 tie(output),
                 [&](Waveform::Type x_wf, Number x_freq, Number x_phase,
                     Number x_pw,
                     Waveform::Type y_wf, Number y_freq, Number y_phase,
                     Number y_pw,
                     Waveform::Type z_wf, Number z_freq, Number z_phase,
                     Number z_pw,
                     Number points, Number closed,
                     Frame& output)
  {
    Number x_theta = x_phase;
    Number y_theta = y_phase;
    Number z_theta = z_phase;

    // Special cases to fix phase difference - waveform library starts
    // at zero-crossing point for saw, with a discontinuity midway,
    // but we want a single smooth line
    if (x_wf == Waveform::Type::saw) x_theta += 0.5;
    if (y_wf == Waveform::Type::saw) y_theta += 0.5;
    if (z_wf == Waveform::Type::saw) z_theta += 0.5;

    for(auto i=0; i<points; i++)
    {
      // Note wrapped before because phase fix can put them over 1.0
      if (x_theta >= 1) x_theta -= floor(x_theta);
      auto x = Waveform::get_value(x_wf, x_pw, x_theta)/2;
      x_theta += x_freq/points;

      if (y_theta >= 1) y_theta -= floor(y_theta);
      auto y = Waveform::get_value(y_wf, y_pw, y_theta)/2;
      y_theta += y_freq/points;

      if (z_theta >= 1) z_theta -= floor(z_theta);
      auto z = Waveform::get_value(z_wf, z_pw, z_theta)/2;
      z_theta += z_freq/points;

      // Double first point with extra blank to start
      if (!i) output.points.push_back(Point(x, y, z));

      // Main lit points
      output.points.push_back(Point(x, y, z, Colour::white));
    }

    // Optionally close the frame back to starting point, but lit
    if (closed && points)
      output.points.push_back(Point(output.points[0], Colour::white));
  });
}

Dataflow::SimpleModule module
{
  "figure",
  "Figure",
  "vector",
  {},
  {
    { "x-wave",        &Figure::x_waveform    },
    { "x-freq",        &Figure::x_freq        },
    { "x-phase",       &Figure::x_phase       },
    { "x-pulse-width", &Figure::x_pulse_width },

    { "y-wave",        &Figure::y_waveform    },
    { "y-freq",        &Figure::y_freq        },
    { "y-phase",       &Figure::y_phase       },
    { "y-pulse-width", &Figure::y_pulse_width },

    { "z-wave",        &Figure::z_waveform    },
    { "z-freq",        &Figure::z_freq        },
    { "z-phase",       &Figure::z_phase       },
    { "z-pulse-width", &Figure::z_pulse_width },

    { "points",        &Figure::points        },
    { "closed",        &Figure::closed        }
  },
  {
    { "output",  &Figure::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Figure, module)

