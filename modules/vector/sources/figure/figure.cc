//==========================================================================
// ViGraph dataflow module: vector/sources/figure/figure.cc
//
// Generic abstract figure source, usable for lines, circles, waves,
// Lissajous etc.
//
// <figure> attributes:
//    points: number of points per frame (default 100)
//    closed: whether to add a final point equal to first
//   <x> <y> axis sub-elements have attributes:
//    pos:  centre position, 0..1 (default 0.5)
//    wave: waveform: none, saw, sine, square, triangle (default none)
//    freq: wave iterations per frame (default 1)
//    phase: wave phase offset, degrees/360 (0..1) (default 0)
//    scale: wave amplitude, 0..1 (default 1)
//
// Point beam:
// <figure id="beam" points="1"/>
//
// Horizontal line:
// <figure id="line" points="10">
//   <x wave="saw"/>
// </figure>
//
// Circle:
// <figure id="circle">
//   <x wave="sin"/>
//   <y wave="sin" phase="0.25"/>
// </figure>
//
// Lissajous:
// <figure id="flower" points="500">
//   <x wave="sin" freq="5"/>
//   <y wave="sin" freq="3"/>
// </figure>
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"
#include <cmath>

namespace {

//==========================================================================
// Figure source
class FigureSource: public Source
{
public:
  // Waveform
  enum class Waveform
  {
    none,
    saw,
    sin,
    square,
    triangle,
    random
  };

  // Axis wave parameters - X and Y
  double x_pos{0.0};
  Waveform x_waveform{Waveform::none};
  double x_freq{1.0};
  double x_phase{0.0};
  double x_scale{1.0};

  double y_pos{0.0};
  Waveform y_waveform{Waveform::none};
  double y_freq{1.0};
  double y_phase{0.0};
  double y_scale{1.0};

  bool closed{false};

  // Overall figure parameters
  static const int default_points = 100;
  int points{default_points};

private:
  // Calculated waves
  vector<coord_t> x_wave;
  vector<coord_t> y_wave;

  // Internals
  string get_waveform_name(Waveform waveform) const;
  void set_waveform(Waveform& waveform, const string& wave);
  void calculate_waveform(double pos, Waveform waveform, double freq,
                          double phase, double scale,
                          vector<coord_t>& wave);

  // Source/Element virtuals
  void update() override;
  void setup() override;
  void tick(const TickData& td) override;

public:
  FigureSource(const Dataflow::Module *module, const XML::Element& config):
    Source(module, config) {}

  // Property getter/setters
  string get_waveform_x() const { return get_waveform_name(x_waveform); }
  void set_waveform_x(const string& wave) { set_waveform(x_waveform, wave); }
  string get_waveform_y() const { return get_waveform_name(y_waveform); }
  void set_waveform_y(const string& wave) { set_waveform(y_waveform, wave); }
};

//--------------------------------------------------------------------------
// Get waveform name for an axis
string FigureSource::get_waveform_name(Waveform waveform) const
{
  switch (waveform)
  {
    case Waveform::none:     return "none";
    case Waveform::saw:      return "saw";
    case Waveform::sin:      return "sin";
    case Waveform::square:   return "square";
    case Waveform::triangle: return "triangle";
    case Waveform::random:   return "random";
  }
}

//--------------------------------------------------------------------------
// Get waveform name for an axis
void FigureSource::set_waveform(Waveform& waveform, const string& wave)
{
  if (wave.empty() || wave=="none")
    waveform = Waveform::none;
  else if (wave=="saw")
    waveform = Waveform::saw;
  else if (wave=="sin")
    waveform = Waveform::sin;
  else if (wave=="square")
    waveform = Waveform::square;
  else if (wave=="triangle")
    waveform = Waveform::triangle;
  else if (wave=="random")
    waveform = Waveform::random;
  else
  {
    Log::Error log;
    log << "Unknown waveform type " << wave << " in figure '" << id << "'\n";
    waveform = Waveform::none;
  }
}

//--------------------------------------------------------------------------
// Precalculate a waveform
void FigureSource::calculate_waveform(double pos, Waveform waveform,
                                      double freq, double phase, double scale,
                                      vector<coord_t>& wave)
{
  // Special case for 'none', for speed - all points are the same
  if (waveform == Waveform::none)
  {
    wave.assign(points, pos);
  }
  else
  {
    // Generic solution
    wave.clear();
    wave.reserve(points);

    for(int i=0; i<points; i++)
    {
      // Proportion of wave (0..freq)
      double theta = phase + freq*i/points;
      double theta1 = theta - floor(theta); // Repeating 0..1

      double offset;  // Offset from baseline (-0.5 .. 0.5)

      switch (waveform)
      {
        case Waveform::none:  // but handled above
          offset = 0.0;
        break;

        case Waveform::saw:
          offset = theta1 - 0.5;
        break;

        case Waveform::sin:
          offset = sin(theta1*2*pi)/2;
        break;

        case Waveform::square:
          offset = theta1 >= 0.5 ? 0.5 : -0.5;
        break;

        case Waveform::triangle:
          offset = (theta1 < 0.5 ? theta1 : 1-theta1)*2-0.5;
        break;

        case Waveform::random:
          offset = (double)rand() / RAND_MAX - 0.5;
        break;
      }

      wave.push_back(pos + scale * offset);
    }
  }
}

//--------------------------------------------------------------------------
// Setup after automatic configuration
void FigureSource::setup()
{
  // Set waveforms
  calculate_waveform(x_pos, x_waveform, x_freq, x_phase, x_scale, x_wave);
  calculate_waveform(y_pos, y_waveform, y_freq, y_phase, y_scale, y_wave);
}

//--------------------------------------------------------------------------
// Update after property set
void FigureSource::update()
{
  setup();
}

//--------------------------------------------------------------------------
// Generate a frame
void FigureSource::tick(const TickData& td)
{
  Frame *frame = new Frame(td.t);

  // Blank to start
  if (points)
    frame->points.push_back(Point(x_wave[0], y_wave[0]));

  // Fill with lit points
  for(int i=0; i<points; i++)
    frame->points.push_back(Point(x_wave[i], y_wave[i], Colour::white));

  // Optionally close the frame back to starting point, but lit
  if (closed && points)
    frame->points.push_back(Point(frame->points[0], Colour::white));

  // Send to output
  send(frame);
}

Dataflow::Module module
{
  "figure",
  "Figure",
  "2D Abstract figure made from X,Y waves",
  "vector",
  {
    { "points", { "Number of points", Value::Type::number,
                  &FigureSource::points, true } },
    { "closed", { "Whether the path is closed", Value::Type::boolean,
                  &FigureSource::closed, true } },
    { "x.wave",  { "Waveform on X axis", Value::Type::choice,
                   { &FigureSource::get_waveform_x,
                     &FigureSource::set_waveform_x },
                   { "none", "saw", "sin", "square", "triangle", "random" } } },
    { "x.pos",   { "Base position on X axis", Value::Type::number,
                   &FigureSource::x_pos, true } },
    { "x.freq",  { "Frequency for X axis", Value::Type::number,
                   &FigureSource::x_freq, true } },
    { "x.phase", { "Phase (0..1) on X axis", Value::Type::number,
                   &FigureSource::x_phase, true } },
    { "x.scale", { { "Scale of X axis", "1.0" }, Value::Type::number,
                   &FigureSource::x_scale, true } },
    { "y.wave",  { "Waveform on Y axis", Value::Type::choice,
                   { &FigureSource::get_waveform_y,
                     &FigureSource::set_waveform_y },
                   { "none", "saw", "sin", "square", "triangle", "random" } } },
    { "y.pos",   { "Base position on Y axis", Value::Type::number,
                   &FigureSource::y_pos, true } },
    { "y.freq",  { "Frequency for Y axis", Value::Type::number,
                   &FigureSource::y_freq, true } },
    { "y.phase", { "Phase (0..1) on Y axis", Value::Type::number,
                   &FigureSource::y_phase, true } },
    { "y.scale", { { "Scale of Y axis", "1.0" }, Value::Type::number,
                     &FigureSource::y_scale, true } }
  },
  {},  // no inputs
  { "VectorFrame" }  // outputs
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(FigureSource, module)

