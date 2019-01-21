//==========================================================================
// ViGraph dataflow module:
//   vector/services/collision-detector/collision-detector.cc
//
// Spots collisions between frames sent by <collision-detect> elements, and
// triggers collision callbacks on all parties involved.
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"
#include "../../vector-services.h"

namespace {

//==========================================================================
// Collision Detector implementation
class CollisionDetectorImpl: public Dataflow::Service, public CollisionDetector
{
  struct Group
  {
    map<CollisionObserver *, Rectangle> bounding_boxes;
  };

  mutable MT::Mutex groups_mutex;
  map<string, Group> groups;

  // Service interface
  void tick(const TickData& td) override;

 public:
  // Construct
  CollisionDetectorImpl(const Dataflow::Module *module,
                        const XML::Element& config):
    Service(module, config)
  {}

  // Check a frame for collision for the given element ID
  void check_for_collision(const string& group,
                           CollisionObserver *obs,
                           Frame *frame) override;
};

//--------------------------------------------------------------------------
// Tick
void CollisionDetectorImpl::tick(const TickData&)
{
  // Clear all boxes for this tick
  MT::Lock lock(groups_mutex);
  for(auto& it: groups)
    it.second.bounding_boxes.clear();
}

//--------------------------------------------------------------------------
// Check a frame for collision for the given element ID
void CollisionDetectorImpl::check_for_collision(const string& group_id,
                                                CollisionObserver *obs,
                                                Frame *frame)
{
  MT::Lock lock(groups_mutex);
  Group& group = groups[group_id];  // Create if not present

  // Get bounding box of frame
  Rectangle bb;
  bb.become_bounding_box(frame->points);

  // Check against existing BBs
  for(const auto& bbit: group.bounding_boxes)
  {
    if (bbit.second.overlaps(bb))
    {
      // Call both parties' observers
      obs->collided();
      bbit.first->collided();
    }
  }

  // Add to BB list for group
  group.bounding_boxes[obs] = bb;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "collision-detector",
  "Collision Detector",
  "Collision detection service, provides tracking of objects in collision groups, "
    " for use with Collision Detect filters",
  "vector",
  {} // no properties
};

} // anon

VIGRAPH_ENGINE_SERVICE_MODULE_INIT(CollisionDetectorImpl, module)
