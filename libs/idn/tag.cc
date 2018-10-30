//==========================================================================
// ViGraph IDN stream library: tag.cc
//
// IDN configuration tag implementation
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-idn.h"

namespace ViGraph { namespace IDN {

//-----------------------------------------------------------------------
// Construct from uint16_t word
Tag::Tag(uint16_t word)
{
  category    = (word >> 12) & 0x0f;
  subcategory = (word >>  8) & 0x0f;
  identifier  = (word >>  4) & 0x0f;
  parameter   = (word      ) & 0x0f;
}

//-----------------------------------------------------------------------
// Convert to uint16_t word
uint16_t Tag::to_word() const
{
  return (category << 12) | (subcategory << 8) | (identifier << 4) | parameter;
}


}} // namespaces
