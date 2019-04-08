//==========================================================================
// ViGraph dataflow machines: graph.cc
//
// Routes data between senders and receivers
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-dataflow.h"
#include "ot-log.h"

namespace ViGraph { namespace Dataflow {

//--------------------------------------------------------------------------
// Register for frame data on the given tag
void Router::register_receiver(const string& tag, Receiver *receiver)
{
  receivers[tag].push_back(receiver);
}

//--------------------------------------------------------------------------
// Deregister a receiver for all tags
void Router::deregister_receiver(Receiver *receiver)
{
  for(auto& it: receivers)
    it.second.remove(receiver);
}

//--------------------------------------------------------------------------
// Get a list of elements subscribed to the given tag
list<Element *> Router::get_receivers(const string& tag)
{
  list<Element *> els;
  const auto it = receivers.find(tag);
  if (it != receivers.end())
  {
    for (const auto it2: it->second)
    {
      auto e = dynamic_cast<Element *>(it2);
      if (e) els.push_back(e);
    }
  }

  return els;
}

//--------------------------------------------------------------------------
// Send frame data on the given tag
void Router::send(const string& tag, DataPtr data)
{
  const auto it = receivers.find(tag);
  if (it != receivers.end())
  {
    for (const auto it2: it->second)
      it2->receive(data);
  }
}


}} // namespaces
