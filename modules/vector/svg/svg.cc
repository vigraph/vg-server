//==========================================================================
// ViGraph dataflow module: vector/svg/svg.cc
//
// SVG frame source
//
// Copyright (c) 2018-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include "vg-svg.h"

namespace {

//==========================================================================
// SVG source
class SVGSource: public SimpleElement
{
private:
  SVG::Path path;
  vector<Point> points;

  // Source/Element virtuals
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;

  // Clone
  SVGSource *create_clone() const override
  {
    return new SVGSource{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Settings
  Setting<string> path_data;
  Setting<string> file;
  Setting<double> precision{SVG::default_precision};
  Setting<bool>   normalise{true};

  // Output
  Output<Frame> output;
};

//--------------------------------------------------------------------------
// Setup
void SVGSource::setup(const SetupContext& context)
{
  Log::Streams log;

  try
  {
    if (!path_data.get().empty())
    {
      path.read(path_data);
    }
    else
    {
      // Look for file
      if (file.get().empty())
      {
        log.error << "No path or file in 'svg'\n";
        return;
      }

      const auto fpath = context.get_file_path(file.get());
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
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  // All 'samples' are the same
  sample_iterate(td, nsamples, {}, {}, tie(output),
                 [&](Frame& output)
  {
    output.points = points;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "svg",
  "SVG image",
  "vector",
  {
    { "path",      &SVGSource::path_data },
    { "file",      &SVGSource::file      },
    { "precision", &SVGSource::precision },
    { "normalise", &SVGSource::normalise }
  },
  {},
  {
    { "output",    &SVGSource::output    }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SVGSource, module)
