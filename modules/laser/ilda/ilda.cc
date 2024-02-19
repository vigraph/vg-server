//==========================================================================
// ViGraph dataflow module: laser/sources/ilda/ilda.cc
//
// ILDA animation source
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector/vector-module.h"
#include "vg-ilda.h"
#include <fstream>

namespace {

const double default_frame_rate = 25.0;

//==========================================================================
// Figure source
class ILDASource: public SimpleElement
{
private:
  ILDA::Animation animation;

  // Source/Element virtuals
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;

  // Clone
  ILDASource *create_clone() const override
  {
    return new ILDASource{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Settings
  Setting<string> file;
  Setting<Number> frame_rate{default_frame_rate};
  Setting<bool> loop{true};

  // Output
  Output<Frame> output;
};

//--------------------------------------------------------------------------
// Setup
void ILDASource::setup(const SetupContext& context)
{
  SimpleElement::setup(context);
  Log::Streams log;

  const auto filename = file.get();
  if (filename.empty())
  {
    log.error << "No file in 'ilda'\n";
    return;
  }

  // Read from file
  File::Path fpath = context.get_file_path(file.get());
  ifstream in(fpath.c_str());
  if (!in)
  {
    log.error << "Can't read ILDA file " << fpath << endl;
    return;
  }

  ILDA::Reader reader(in);
  reader.read(animation);
  if (!animation.frames.size())
  {
    log.error << "Empty ILDA animation in " << fpath << endl;
    return;
  }

  log.summary << "Loaded ILDA animation " << filename << " with "
              << animation.frames.size() << " frames\n";
}

//--------------------------------------------------------------------------
// Generate a frame
void ILDASource::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());

  // !!! For now, just take first frame - implement animation later
  sample_iterate(td, nsamples, {}, {}, tie(output),
                 [&](Frame& output)
  {
    if (animation.frames.size())
      output.points = animation.frames[0].points;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "ilda",
  "ILDA animation",
  "laser",
  {
    { "file",       &ILDASource::file },
    { "frame-rate", &ILDASource::frame_rate },
    { "loop",       &ILDASource::loop }
  },
  {}, // no inputs
  {
    { "output",     &ILDASource::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ILDASource, module)

