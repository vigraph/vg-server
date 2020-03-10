//==========================================================================
// ViGraph dataflow module: vector/collision-detector/collision-detector.cc
//
// Collision detector
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include <cmath>

namespace {

//==========================================================================
// CollisionDetector
class CollisionDetector: public SimpleElement
{
private:
  Frame last_subject;
  Frame last_object;

  // Element virtuals
  void tick(const TickData& td) override;

  // Custom ready() so we can live in a loop
  bool ready() const override { return true; }

  // Read the input before it is reset, for next time
  // Note we do the collision calculation one tick behind, to allow for
  // the common case where the collision affects the subject, without
  // needing a trigger/memory.
  void reset() override
  {
    const auto& subj = subject.get_buffer();
    if (subj.size()) last_subject = subj.back();

    const auto& obj = object.get_buffer();
    if (obj.size()) last_object = obj.back();

    Element::reset();
  }

  // We control our own input sample rate
  void update_sample_rate() override {}

  // Capture sample rate on setup
  void setup(const SetupContext& context) override
  {
    SimpleElement::setup(context);
    auto tick_interval = context.get_engine().get_tick_interval().seconds();
    if (tick_interval)
    {
      subject.set_sample_rate(1.0/tick_interval);
      object.set_sample_rate(1.0/tick_interval);
    }
  }

  // Clone
  CollisionDetector *create_clone() const override
  {
    return new CollisionDetector{module};
  }

  // Internal
  void get_bounding_boxes(const Frame& frame, vector<Rectangle>& bbs);

public:
  using SimpleElement::SimpleElement;

  // Input
  Input<Frame> subject;
  Input<Frame> object;

  // Output
  Output<Trigger> collided;
};

//--------------------------------------------------------------------------
// Tick data
void CollisionDetector::tick(const TickData& td)
{
  bool collision = false;

  // Accumulate bounding boxes of subjects
  vector<Rectangle> subject_bbs;
  get_bounding_boxes(last_subject, subject_bbs);

  // Accumulate bounding boxes of objects
  vector<Rectangle> object_bbs;
  get_bounding_boxes(last_object, object_bbs);

  // Now N*M match for overlap
  for(const auto& sbb: subject_bbs)
    for(const auto& obb: object_bbs)
      if (sbb.overlaps(obb))
      {
        collision = true;
        goto done;  // Yeah I know, but no labelled break!
      }
  done:

  const auto sample_rate = collided.get_sample_rate();
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {}, {},
                 tie(collided),
                 [&](Trigger& collided)
  {
    collided = collision?1:0;
  });

}

void CollisionDetector::get_bounding_boxes(const Frame& frame,
                                           vector<Rectangle>& bbs)
{
  // Scan for sections bounded by blanks
  auto start = frame.points.begin();
  auto last = start;
  for(auto it=start; it!=frame.points.end(); it++)
  {
    if (it->is_blanked())
    {
      if (last != start)
      {
        Rectangle bb;
        bb.become_bounding_box(start, last);
        bbs.push_back(bb);
      }
      start = it;
    }
    last = it;
  }

  // ... and final section
  if (last != start)
  {
    Rectangle bb;
    bb.become_bounding_box(start, last);
    bbs.push_back(bb);
  }
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "collision-detector",
  "Collision detector",
  "vector",
  {},
  {
    { "subject", &CollisionDetector::subject },
    { "object",  &CollisionDetector::object  }
  },
  {
    { "collided", &CollisionDetector::collided }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(CollisionDetector, module)
