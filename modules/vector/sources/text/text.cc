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
  string text;
  static constexpr double default_precision = 0.1;
  double precision{default_precision};
  vector<Point> points;

  // Source/Element virtuals
  void configure(const XML::Element& config) override;
  void tick(Dataflow::timestamp_t t) override;

public:
  TextSource(const Dataflow::Module *module, const XML::Element& config):
    Element(module, config), Source(module, config) {}
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <text font="arial.ttf" precision="0.1">Hello, world!</text>
void TextSource::configure(const XML::Element& config)
{
  Log::Streams log;
  text = *config;
  precision = config.get_attr_real("precision", default_precision);

  //  const auto& font = config["font"];
}

//--------------------------------------------------------------------------
// Generate a frame
void TextSource::tick(Dataflow::timestamp_t t)
{
  Frame *frame = new Frame(t);
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
    { "precision", { {"Precision for character generation","0.1"},
          Value::Type::number}}
  },
  {}, // no inputs
  { "VectorFrame" }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(TextSource, module)
