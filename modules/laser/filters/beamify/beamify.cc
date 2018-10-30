//==========================================================================
// ViGraph dataflow module: laser/filters/beamify/beamify.cc
//
// Filter which replicates points to create beams within a figure
//
// <beamify span="10" multiple="5"/>
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../vector/vector-module.h"

namespace {

//==========================================================================
// Beamify filter
class BeamifyFilter: public FrameFilter
{
  int span;      // Number of points between beams
  int n;         // Number of points to replicate in the beam

  // Filter/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void accept(FramePtr frame) override;

public:
  // Construct
  BeamifyFilter(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
BeamifyFilter::BeamifyFilter(const Dataflow::Module *module,
                             const XML::Element& config):
  Element(module, config), FrameFilter(module, config)
{
  span = config.get_attr_int("span");
  n = config.get_attr_int("n");
}

//--------------------------------------------------------------------------
// Set a control property
void BeamifyFilter::set_property(const string& property, const SetParams& sp)
{
       if (property == "span") update_prop_int(span, sp);
  else if (property == "n")    update_prop_int(n, sp);
}

//--------------------------------------------------------------------------
// Process some data
void BeamifyFilter::accept(FramePtr frame)
{
  // Insert 'n' points every 'span'
  if (span) for(size_t i=0; i<frame->points.size(); i+=span+n)
    frame->points.insert(frame->points.begin()+i, n, frame->points[i]);
  send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "beamify",
  "Beamify",
  "Add bright points to a continuous frame",
  "vector",
  {
    { "span", { "Distance between points", Value::Type::number, "@span", true}},
    { "n",    { "Number of points to add", Value::Type::number, "@n", true } }
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(BeamifyFilter, module)
