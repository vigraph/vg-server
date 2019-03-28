//==========================================================================
// ViGraph dataflow module: vector/sources/text/text.cc
//
// Text source
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"
#include "vg-font.h"

namespace {

//==========================================================================
// Text source
class TextSource: public Dataflow::Source
{
public:
  string text;
  static constexpr double default_precision = 0.1;
  double precision{default_precision};

private:
  vector<Point> points;

  // Source/Element virtuals
  void tick(const TickData& td) override;

public:
  using Source::Source;
};

//--------------------------------------------------------------------------
// Generate a frame
void TextSource::tick(const TickData& td)
{
  Frame *frame = new Frame(td.t);
  frame->points = points;
  send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "text",
  "Text",
  "Text display",
  "vector",
  {
    { "", { "Text to use", Value::Type::text, &TextSource::text, true } },
    { "precision", { {"Precision for character generation","0.1"},
                     Value::Type::number, &TextSource::precision, true } }
  },
  {}, // no inputs
  { "VectorFrame" }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(TextSource, module)
