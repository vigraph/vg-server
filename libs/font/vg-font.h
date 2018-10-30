//==========================================================================
// ViGraph Font library: vg-font.h
//
// C++ wrapper for FreeType, providing a central font face cache plus
// render to vector
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VG_FONT_H
#define __VG_FONT_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include "vg-geometry.h"
#include <map>
#include "ot-mt.h"

namespace ViGraph { namespace Font {

using namespace std;
using namespace ViGraph::Geometry;
using namespace ObTools;

// -------------------------------------------------------------------------
// Font face
class Face
{
  FT_Face handle{0};
  mutable MT::Mutex mutex;  // FT_Face is not threadsafe

 public:
  // Constructors
  Face() {}
  Face(FT_Face _handle): handle(_handle) {}
  Face(const Face& o): handle(o.handle) {}

  // Check for validity
  bool operator!() const { return !handle; }

  // Compare (for cache test)
  bool operator==(const Face& o) const { return handle == o.handle; }

  // Render to points at the given size
  // Render to points
  // precision is currently the increment of 't', hence 1/number of points
  static constexpr double default_precision{0.1};  // 10 points
  void render(const string& text, coord_t height,
              vector<Point>& points, double precision = default_precision) const;
};

// -------------------------------------------------------------------------
// Font cache
// Also holds the FreeType library handle - use as singleton
class Cache
{
  FT_Library library;
  MT::Mutex mutex;
  map<string, FT_Face> faces;  // filename -> face

 public:
  // Constructors
  Cache();

  // Check for goodness
  bool operator!() { return !library; }

  // Read (and cache) a face
  Face load(const string& filename);

  // Destructor
  ~Cache();
};


//==========================================================================
}} //namespaces
#endif // !__VG_FONT_H
