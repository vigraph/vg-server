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
// Flatten into a single Rectangle, which is prefilled with the given colour
// Sets the size of the rectangle to the group's bounding box, plus the origin
void Group::compose(Rectangle& result, const Colour::RGB& background) const
{
  const auto bb = bounding_box();
  result = Rectangle(bb.p1.x, bb.p1.y);  // Include origin
  result.fill(background);

  // Create vector of pointers to sort
  vector<const Item *> sort_items;
  for(const auto& item: items)
    sort_items.push_back(&item);

  // Sort in z order
  sort(sort_items.begin(), sort_items.end(),
       [] (const Item *a, const Item *b)
       { return a->pos.z < b->pos.z; });

  // Blit them back to front
  for(const auto item: sort_items)
    item->rect.blit(item->pos, result);
}

}} // namespaces
