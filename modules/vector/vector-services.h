//==========================================================================
// ViGraph vector modules: vector-services.h
//
// Definitions of shared services for vector modules
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_VECTOR_SERVICES_H
#define __VIGRAPH_VECTOR_SERVICES_H

namespace ViGraph { namespace Module { namespace Vector {

//==========================================================================
// Collision Detector interface
class CollisionDetector
{
 public:
  struct CollisionObserver
  {
    virtual void collided() = 0;
  };

  // Construct
  CollisionDetector() {}

  // Check a frame for collision for the given observer
  virtual void check_for_collision(const string& group,
                                   CollisionObserver *obs,
                                   Frame *frame) = 0;
};

//==========================================================================
// Router interface
class Router
{
 public:
  struct Receiver
  {
    virtual void receive(FramePtr frame) = 0;
  };

  // Construct
  Router() {}

  // Register for frame data on the given tag
  virtual void register_receiver(const string& tag, Receiver *receiver) = 0;

  // Deregister a receiver for all tags
  virtual void deregister_receiver(Receiver *receiver) = 0;

  // Send frame data on the given tag
  virtual void send(const string& tag, FramePtr frame) = 0;

  // Virtual destructor
  virtual ~Router() {}
};

//==========================================================================
}}} // namespaces
#endif // !__VIGRAPH_VECTOR_SERVICES_H
