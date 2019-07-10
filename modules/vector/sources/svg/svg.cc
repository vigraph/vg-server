//==========================================================================
// ViGraph dataflow module: vector/sources/svg/svg.cc
//
// SVG frame source
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"
#include "vg-svg.h"

namespace {

//==========================================================================
// SVG source
class SVGSource: public Source
{
public:
  string path_data;
  string file;
  double precision = SVG::Path::default_precision;
  bool normalise = true;

private:
  SVG::Path path;
  vector<Point> points;

  // Source/Element virtuals
  void setup() override;
  void tick(const TickData& td) override;

public:
  using Source::Source;
};

//--------------------------------------------------------------------------
// Setup
void SVGSource::setup()
{
  Log::Streams log;

  try
  {
    if (!path_data.empty())
    {
      path.read(path_data);
    }
    else
    {
      // Look for file
      if (file.empty())
      {
        log.error << "No path or file in <svg>\n";
        return;
      }

      File::Path fpath(file);
      XML::Configuration cfg(fpath.str(), log.error);
      if (!cfg.read())
      {
        log.error << "Can't read SVG file " << fpath << endl;
        return;
      }

      // Get all paths from XML
      XML::Element& root = cfg.get_root();
      path.read(root);
    }
  }
  catch (const runtime_error& e)
  {
    log.error << "Failed to read SVG: " << e.what() << endl;
    return;
  }

  log.summary << "Loaded SVG animation with "
              << path.segments.size() << " segments\n";
  log.detail << "Curve precision: " << precision << endl;
  if (!normalise) log.detail << "No normalisation\n";

  // Render to points immediately, since it doesn't change
  path.render(points, precision, normalise);
  log.detail << "SVG generated " << points.size() << " points\n";
}

//--------------------------------------------------------------------------
// Generate a frame
void SVGSource::tick(const TickData& td)
{
  Frame *frame = new Frame(td.t);
  frame->points = points;
  send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "svg",
  "SVG image",
  "SVG vector graphic image",
  "vector",
  {
    { "path", { "SVG path data (as if in <path d=...>)", Value::Type::text,
                &SVGSource::path_data, false } },
    { "file", { "Filename of SVG file", Value::Type::file,
                &SVGSource::file, false } },
    { "precision", { "Curve precision", Value::Type::number,
                     &SVGSource::precision, false } },
    { "normalise", { "Scale to unit square", Value::Type::boolean,
                     &SVGSource::normalise, false } },
  },
  {}, // no inputs
  { "VectorFrame" }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SVGSource, module)
