//==========================================================================
// ViGraph dataflow module: filters/collision-detect.cc
//
// Collision detection filter
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"
#include "../../vector-services.h"

namespace {

const string default_group_name = "default";

//==========================================================================
// CollisionDetect filter
class CollisionDetectFilter: public FrameFilter, public Dataflow::ControlImpl,
                             public CollisionDetector::CollisionObserver
{
public:
  string group_name = default_group_name;

private:
  shared_ptr<CollisionDetector> detector;
  unsigned long tick_collisions = 0;

  // Filter/Element virtuals
  void setup() override;
  void accept(FramePtr frame) override;
  void pre_tick(const TickData& td) override;

  // Add control JSON
  JSON::Value get_json(const string& path) const override
  { JSON::Value json=Element::get_json(path); add_to_json(json); return json; }

  // CollisionObserver virtual
  void collided() override;

public:
  // Construct
  CollisionDetectFilter(const Dataflow::Module *module,
                        const XML::Element& config):
    FrameFilter(module, config),
    ControlImpl(module, config, true)
  {}
};

//--------------------------------------------------------------------------
// Setup
void CollisionDetectFilter::setup()
{
  auto& engine = graph->get_engine();
  detector = engine.get_service<CollisionDetector>("collision-detector");
}

//--------------------------------------------------------------------------
// Process some data
void CollisionDetectFilter::accept(FramePtr frame)
{
  // Pass frame to collision detector
  if (detector) detector->check_for_collision(group_name, this, frame.get());

  // Pass it on
  Generator::send(frame);
}

//--------------------------------------------------------------------------
// Handle a collision
void CollisionDetectFilter::collided()
{
  ++tick_collisions;
}

//--------------------------------------------------------------------------
// Send any collisions
void CollisionDetectFilter::pre_tick(const TickData&)
{
  while (tick_collisions)
  {
    trigger();
    --tick_collisions;
  }
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "collision-detect",
  "Collision Detector",
  "Detects collisions with other members of the same group, "
    "and sends triggers if they happen",
  "vector",
  {
    { "group", { "Group name", Value::Type::number,
                 &CollisionDetectFilter::group_name, true } }
  },
  { { "trigger", { "Trigger output", "trigger", Value::Type::trigger }}},
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(CollisionDetectFilter, module)
