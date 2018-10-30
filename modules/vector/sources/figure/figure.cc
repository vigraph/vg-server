//==========================================================================
// ViGraph dataflow module: vector/sources/figure/figure.cc
//
// Generic abstract figure source, usable for lines, circles, waves,
// Lissajous etc.
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
  struct Axis
  {
    double pos{0.0};
    Waveform waveform{Waveform::none};
    double freq{1.0};
    double phase{0.0};
    double scale{1.0};
  };

  Axis x_axis, y_axis;
  bool closed{false};

  // Overall figure parameters
  static const int default_points = 100;
  int points{default_points};

  // Calculated waves
  vector<coord_t> x_waveform;
  vector<coord_t> y_waveform;

  // Internals
  void calculate_waveform(const Axis& axis, vector<coord_t>& waveform);
  void read_axis(const XML::Element& aconfig, Axis& axis);

  // Source/Element virtuals
  void configure(const XML::Element& config) override;
  void set_property(const string& property, const SetParams& sp) override;
  void tick(timestamp_t t) override;

public:
  FigureSource(const Dataflow::Module *module, const XML::Element& config):
    Element(module, config), Source(module, config) {}
};

//--------------------------------------------------------------------------
// Precalculate a waveform
void FigureSource::calculate_waveform(const Axis& axis,
                                     vector<coord_t>& waveform)
{

  // Special case for 'none', for speed - all points are the same
  if (axis.waveform == Waveform::none)
  {
    waveform.assign(points, axis.pos);
  }
  else
  {
    // Generic solution
    waveform.clear();
    waveform.reserve(points);

    for(int i=0; i<points; i++)
    {
      // Proportion of wave (0..freq)
      double theta = axis.phase + axis.freq*i/points;
      double theta1 = theta - floor(theta); // Repeating 0..1

      double offset;  // Offset from baseline (-0.5 .. 0.5)

      switch (axis.waveform)
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

      waveform.push_back(axis.pos + axis.scale * offset);
    }
  }
}

//--------------------------------------------------------------------------
// Construct from XML:
//   <figure> attributes:
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

// Read an axis
void FigureSource::read_axis(const XML::Element& aconfig, Axis& axis)
{
  axis.pos = aconfig.get_attr_real("pos");
  const string& wave = aconfig["wave"];
  if (wave.empty() || wave=="none")
    axis.waveform = Waveform::none;
  else if (wave=="saw")
    axis.waveform = Waveform::saw;
  else if (wave=="sin")
    axis.waveform = Waveform::sin;
  else if (wave=="square")
    axis.waveform = Waveform::square;
  else if (wave=="triangle")
    axis.waveform = Waveform::triangle;
  else if (wave=="random")
    axis.waveform = Waveform::random;
  else
  {
    Log::Error log;
    log << "Unknown waveform type " << wave << " in figure '" << id << "'\n";
  }

  axis.freq = aconfig.get_attr_real("freq", 1.0);
  axis.phase = aconfig.get_attr_real("phase");
  axis.scale = aconfig.get_attr_real("scale", 1.0);
}

// Configure from XML
void FigureSource::configure(const XML::Element& config)
{
  // General config
  points = config.get_attr_int("points", default_points);
  closed = config.get_attr_bool("closed");

  // Axis config
  read_axis(config.get_child("x"), x_axis);
  read_axis(config.get_child("y"), y_axis);

  // Set waveforms
  calculate_waveform(x_axis, x_waveform);
  calculate_waveform(y_axis, y_waveform);
}

//--------------------------------------------------------------------------
// Set a control property
void FigureSource::set_property(const string& property, const SetParams& sp)
{
  // Assume type is OK from graph construction

  bool u_x{false}, u_y{false};

       if (property == "x.pos")   { update_prop(x_axis.pos, sp);   u_x = true; }
  else if (property == "y.pos")   { update_prop(y_axis.pos, sp);   u_y = true; }
  else if (property == "x.freq")  { update_prop(x_axis.freq, sp);  u_x = true; }
  else if (property == "y.freq")  { update_prop(y_axis.freq, sp);  u_y = true; }
  else if (property == "x.phase") { update_prop(x_axis.phase, sp); u_x = true; }
  else if (property == "y.phase") { update_prop(y_axis.phase, sp); u_y = true; }
  else if (property == "x.scale") { update_prop(x_axis.scale, sp); u_x = true; }
  else if (property == "y.scale") { update_prop(y_axis.scale, sp); u_y = true; }
  else if (property == "points")  { update_prop_int(points, sp);
                                    u_x = u_y = true; }

  if (u_x) calculate_waveform(x_axis, x_waveform);
  if (u_y) calculate_waveform(y_axis, y_waveform);
}

//--------------------------------------------------------------------------
// Generate a frame
void FigureSource::tick(timestamp_t t)
{
  Frame *frame = new Frame(t);

  // Fill with lit points
  for(int i=0; i<points; i++)
    frame->points.push_back(Point(x_waveform[i], y_waveform[i],
                                  Colour::white));

  // Optionally close the frame back to starting point
  if (closed && points) frame->points.push_back(frame->points[0]);

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
    { "points", { "Number of points",
          Value::Type::number } },
    { "closed", { "Whether the path is closed",
          Value::Type::boolean } },
    { "x.wave",  { "Waveform on X axis",
          Value::Type::choice, "x/@wave",
          { "none", "saw", "sin", "square", "triangle", "random" } } },
    { "x.pos",   { "Base position on X axis",
          Value::Type::number, "x/@pos",   true } },
    { "x.freq",  { "Frequency for X axis",
          Value::Type::number, "x/@freq",  true } },
    { "x.phase", { "Phase (0..1) on X axis",
          Value::Type::number, "x/@phase", true } },
    { "x.scale", { { "Scale of X axis", "1.0" },
          Value::Type::number, "x/@scale", true } },
    { "y.wave",  { "Waveform on Y axis",
          Value::Type::choice, "y/@wave",
          { "none", "saw", "sin", "square", "triangle", "random" } } },
    { "y.pos",   { "Base position on Y axis",
          Value::Type::number, "y/@pos",   true } },
    { "y.freq",  { "Frequence for Y axis",
          Value::Type::number, "y/@freq",  true } },
    { "y.phase", { "Phase (0..1) on Y axis",
          Value::Type::number, "y/@phase", true } },
    { "y.scale", { { "Scale of Y axis", "1.0" },
          Value::Type::number, "y/@scale", true } }
  },
  {},  // no inputs
  { "VectorFrame" }  // outputs
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(FigureSource, module)

