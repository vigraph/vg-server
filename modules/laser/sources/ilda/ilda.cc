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

const double default_frame_rate = 25.0;

//==========================================================================
// Figure source
class ILDASource: public Dataflow::Source
{
public:
  string filename;
  double frame_rate = default_frame_rate;

private:
  ILDA::Animation animation;

  // Source/Element virtuals
  void setup(const File::Directory& base_dir) override;
  void tick(const TickData& td) override;

public:
  using Source::Source;
};

//--------------------------------------------------------------------------
// Setup
void ILDASource::setup(const File::Directory& base_dir)
{
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
void ILDASource::tick(const TickData& td)
{
  Frame *frame = new Frame(td.t);

  size_t nframes = animation.frames.size();

  // Get frame number, looping !!! make looping optional?
  size_t i = static_cast<size_t>(td.t * frame_rate) % nframes;
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
    { "file", { "Filename of ILDA file", Value::Type::file,
                &ILDASource::filename, false } },
    { "frame-rate", { "Frame rate (frames per sec)", Value::Type::number,
                      &ILDASource::frame_rate, true } }
  },
  {}, // no inputs
  { "VectorFrame" }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ILDASource, module)

