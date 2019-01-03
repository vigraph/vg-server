//==========================================================================
// ViGraph dataflow module: laser/sources/ilda/ilda.cc
//
// ILDA animation source
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../vector/vector-module.h"
#include "vg-ilda.h"
#include <fstream>

namespace {

//==========================================================================
// Figure source
class ILDASource: public Dataflow::Source
{
  ILDA::Animation animation;
  double frame_rate;

  static constexpr double default_frame_rate = 25.0;

  // Source/Element virtuals
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override;
  void tick(Dataflow::timestamp_t t) override;

public:
  ILDASource(const Dataflow::Module *module, const XML::Element& config):
    Element(module, config), Source(module, config) {}
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <ilda file="..." frame-rate="25"/>
void ILDASource::configure(const File::Directory& base_dir,
                           const XML::Element& config)
{
  // General config
  string filename = config["file"];
  frame_rate = config.get_attr_real("frame-rate", default_frame_rate);

  // Read from file
  File::Path fpath(base_dir, filename);
  ifstream in(fpath.c_str());
  if (!in) throw runtime_error("Can't read ILDA file "+fpath.str());

  ILDA::Reader reader(in);
  reader.read(animation);
  if (!animation.frames.size()) throw runtime_error("Empty ILDA animation");

  Log::Summary log;
  log << "Loaded ILDA animation " << filename << " with "
      << animation.frames.size() << " frames\n";
}

//--------------------------------------------------------------------------
// Generate a frame
void ILDASource::tick(Dataflow::timestamp_t t)
{
  Frame *frame = new Frame(t);

  size_t nframes = animation.frames.size();

  // Get frame number, looping !!! make looping optional?
  size_t i = static_cast<size_t>(t * frame_rate) % nframes;
  frame->points = animation.frames[i].points;

  // Send to output
  send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "ilda",
  "ILDA animation",
  "ILDA file format animation player",
  "vector",
  {
    { "file", { "Filename of ILDA file", Value::Type::file } },
    { "frame-rate", { { "Frame rate (frames per sec)", "25" },
          Value::Type::number }}
  },
  {}, // no inputs
  { "VectorFrame" }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ILDASource, module)

