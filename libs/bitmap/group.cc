//==========================================================================
// ViGraph bitmap library: group.cc
//
// Implementation of bitmap group operations
// A group is a sparse set of bitmap rectangles, with z-order
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-bitmap.h"
#include <sstream>
#include <algorithm>

namespace ViGraph { namespace Bitmap {

// -------------------------------------------------------------------
// Get the bounding box of all items
Geometry::Rectangle Group::bounding_box() const
{
  Geometry::Rectangle bb;
  auto first{true};
  for(const auto& item: items)
  {
    if (first)
    {
      bb.p0 = bb.p1 = item.pos;
      first = false;
    }
    else
      bb.expand_to_include(item.pos);
    bb.expand_to_include(item.pos+item.rect.size());
  }

  return bb;
}

// -------------------------------------------------------------------
// Flatten into a single Rectangle
// Individual bitmaps will be clipped to the result's size
void Group::compose(Rectangle& result) const
{
  // Create vector of pointers to sort
  vector<const Item *> sort_items;
  for(const auto& item: items)
    sort_items.push_back(&item);

  // Sort in z order
  sort(sort_items.begin(), sort_items.end(),
       [] (const Item *a, const Item *b)
       { return a->pos.z < b->pos.z; });

  auto rw = result.get_width();
  auto rh = result.get_height();

  // Apply them back to front
  for(const auto item: sort_items)
  {
    auto pos = item->pos;
    auto w = item->rect.get_width();
    auto h = item->rect.get_height();

    // Scale positions from unit square around centre of result,
    // positive upwards
    pos.x = (0.5 + pos.x)*rw - w/2;
    pos.y = (0.5 - pos.y)*rh - h/2;

    item->rect.apply(pos, result);
  }
}

}} // namespaces
