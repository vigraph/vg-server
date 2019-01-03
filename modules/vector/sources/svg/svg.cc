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
  SVG::Path path;
  double precision;
  vector<Point> points;

  // Source/Element virtuals
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override;
  void tick(timestamp_t t) override;

public:
  SVGSource(const Dataflow::Module *module, const XML::Element& config):
    Element(module, config), Source(module, config) {}
};

//--------------------------------------------------------------------------
// Construct from XML:
//    <svg path="M0 0 L 1 2..."/>
// or <svg file="picture.svg"/>
void SVGSource::configure(const File::Directory& base_dir,
                          const XML::Element& config)
{
  Log::Streams log;

  try
  {
    const string& path_data = config["path"];
    if (!path_data.empty())
    {
      path.read(path_data);
    }
    else
    {
      // Look for file
      const string& file = config["file"];
      if (file.empty())
      {
        log.error << "No path or file in <svg>\n";
        return;
      }

      File::Path fpath(base_dir, file);
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
  catch (runtime_error e)
  {
    log.error << "Failed to read SVG: " << e.what() << endl;
    return;
  }

  precision = config.get_attr_real("precision", SVG::Path::default_precision);

  log.summary << "Loaded SVG animation with "
              << path.segments.size() << " segments\n";
  log.detail << "Curve precision: " << precision << endl;

  // Render to points immediately, since it doesn't change
  path.render(points, precision, true);  // normalising
  log.detail << "SVG generated " << points.size() << " points\n";
}

//--------------------------------------------------------------------------
// Generate a frame
void SVGSource::tick(timestamp_t t)
{
  Frame *frame = new Frame(t);
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
    { "path", { "SVG path data (as if in <path d=...>)", Value::Type::text } },
    { "file", { "Filename of SVG file", Value::Type::file } }
  },
  {}, // no inputs
  { "VectorFrame" }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SVGSource, module)
