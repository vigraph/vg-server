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
class CollisionDetectFilter: public FrameFilter, public Dataflow::Control,
                             public CollisionDetector::CollisionObserver
{
  string group_name;
  shared_ptr<CollisionDetector> detector;

  // Filter/Element virtuals
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override;
  void accept(FramePtr frame) override;

  // CollisionObserver virtual
  void collided() override;

public:
  // Construct
  CollisionDetectFilter(const Dataflow::Module *module,
                        const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//  <collision-detect/>
CollisionDetectFilter::CollisionDetectFilter(const Dataflow::Module *module,
                                             const XML::Element& config):
  Element(module, config), FrameFilter(module, config),
  Control(module, config, true)  // optional targets
{
  group_name = config.get_attr("group", default_group_name);
}

//--------------------------------------------------------------------------
// Configure from XML (once we have the engine)
void CollisionDetectFilter::configure(const File::Directory&,
                                      const XML::Element&)
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
  Control::send(SetParams{Dataflow::Value{}});  // trigger
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
    { "group", { { "Group name", "default" }, Value::Type::number } }
  },
  { { "", { "Trigger output", "trigger", Value::Type::trigger }}},
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(CollisionDetectFilter, module)
