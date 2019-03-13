//==========================================================================
// ViGraph dataflow module: vector/sources/test/test.cc
//
// Test pattern source
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"

namespace {

//==========================================================================
// Figure source
class TestSource: public Dataflow::Source
{
  static constexpr double default_precision = 0.1;
  double precision{default_precision};

  // Source/Element virtuals
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override;
  void tick(const TickData& td) override;

  // Internals
  void interpolate(const Line& l, vector<Point>& points);

public:
  TestSource(const Dataflow::Module *module, const XML::Element& config):
    Source(module, config) {}
};

//--------------------------------------------------------------------------
// Construct from XML:
//    <test/>
void TestSource::configure(const File::Directory&,
                           const XML::Element& config)
{
  precision = config.get_attr_real("precision", default_precision);
}

//--------------------------------------------------------------------------
// Generate a frame
void TestSource::tick(const TickData& td)
{
  Frame *frame = new Frame(td.t);
  auto& points = frame->points;

  // Square corners
  Point bl(-0.5, -0.5, Colour::white);
  Point tl(-0.5,  0.5, Colour::white);
  Point tr( 0.5,  0.5, Colour::white);
  Point br( 0.5, -0.5, Colour::white);

  // 4 edges
  interpolate(Line(bl, tl), points);
  interpolate(Line(tl, tr), points);
  interpolate(Line(tr, br), points);
  interpolate(Line(br, bl), points);

  // Two diagonals
  interpolate(Line(bl, tr), points);
  interpolate(Line(tl, br), points);

  send(frame);
}

//--------------------------------------------------------------------------
// Interpolate a line
void TestSource::interpolate(const Line& l, vector<Point>& points)
{
  // Blank points leading in
  Point p = l.p0;
  p.blank();
  for(int i=0; i<10; i++) points.push_back(p);

  for(double t=precision; t<=1.0+precision/2; t+=precision)
    points.push_back(l.interpolate(t));

  // Dwell on last point
  for(int i=0; i<10; i++) points.push_back(l.p1);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "test",
  "Test pattern",
  "Test pattern generator",
  "vector",
  {
    { "precision", { {"Precision for line generation","0.1"}, Value::Type::number}}
  },
  {}, // no inputs
  { "VectorFrame" }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(TestSource, module)

