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
//    ?-pos:  centre position, 0..1 (default 0.5)
//    ?-scale: wave amplitude, 0..1 (default 1)
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
#include "../../core/core-module.h"
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
  Input<double> x_freq{1.0};
  Input<double> x_phase{0.0};
  Input<double> x_pulse_width{0.5};
  Input<double> x_pos{0.0};
  Input<double> x_scale{1.0};

  Input<Waveform::Type> y_waveform;
  Input<double> y_freq{1.0};
  Input<double> y_phase{0.0};
  Input<double> y_pulse_width{0.5};
  Input<double> y_pos{0.0};
  Input<double> y_scale{1.0};

  Input<Waveform::Type> z_waveform;
  Input<double> z_freq{1.0};
  Input<double> z_phase{0.0};
  Input<double> z_pulse_width{0.5};
  Input<double> z_pos{0.0};
  Input<double> z_scale{1.0};

  Input<double> points{default_points};
  Input<double> closed{0.0};

  // Outputs
  Output<Frame> output;
};

//--------------------------------------------------------------------------
// Generate a fragment
void Figure::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(nsamples, {},
                 tie(x_waveform, x_freq, x_phase, x_pulse_width, x_pos, x_scale,
                     y_waveform, y_freq, y_phase, y_pulse_width, y_pos, y_scale,
                     z_waveform, z_freq, z_phase, z_pulse_width, z_pos, z_scale,
                     points, closed),
                 tie(output),
                 [&](Waveform::Type x_wf, double x_freq, double x_phase,
                     double x_pw, double x_pos, double x_scale,
                     Waveform::Type y_wf, double y_freq, double y_phase,
                     double y_pw, double y_pos, double y_scale,
                     Waveform::Type z_wf, double z_freq, double z_phase,
                     double z_pw, double z_pos, double z_scale,
                     double points, double closed,
                     Frame& output)
  {
    double x_theta = x_phase;
    double y_theta = y_phase;
    double z_theta = z_phase;

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
      auto x = Waveform::get_value(x_wf, x_pw, x_theta)/2*x_scale+x_pos;
      x_theta += x_freq/points;

      if (y_theta >= 1) y_theta -= floor(y_theta);
      auto y = Waveform::get_value(y_wf, y_pw, y_theta)/2*y_scale+y_pos;
      y_theta += y_freq/points;

      if (z_theta >= 1) z_theta -= floor(z_theta);
      auto z = Waveform::get_value(z_wf, z_pw, z_theta)/2*z_scale+z_pos;
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
    { "x-pos",         &Figure::x_pos         },
    { "x-scale",       &Figure::x_scale       },

    { "y-wave",        &Figure::y_waveform    },
    { "y-freq",        &Figure::y_freq        },
    { "y-phase",       &Figure::y_phase       },
    { "y-pulse-width", &Figure::y_pulse_width },
    { "y-pos",         &Figure::y_pos         },
    { "y-scale",       &Figure::y_scale       },

    { "z-wave",        &Figure::z_waveform    },
    { "z-freq",        &Figure::z_freq        },
    { "z-phase",       &Figure::z_phase       },
    { "z-pulse-width", &Figure::z_pulse_width },
    { "z-pos",         &Figure::z_pos         },
    { "z-scale",       &Figure::z_scale       },

    { "points",        &Figure::points        },
    { "closed",        &Figure::closed        }
  },
  {
    { "output",  &Figure::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Figure, module)

