//==========================================================================
// ViGraph Font library: cache.cc
//
// Cache of faces
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-font.h"
#include "ot-log.h"

namespace ViGraph { namespace Font {

using namespace ObTools;

// Constructor - initialise library
Cache::Cache()
{
  auto error = FT_Init_FreeType(&library);
  if (error)
  {
    Log::Error log;
    log << "Failed to initialise FreeType library: " << error << endl;
    return;
  }
}

// Read (and cache) a face
Face Cache::load(const string& filename)
{
  MT::Lock lock(mutex);

  if (faces.find(filename) != faces.end())
    return Face(faces[filename]);

  FT_Face face;
  auto error = FT_New_Face(library, filename.c_str(), 0, &face);
  if (error)
  {
    Log::Error log;
    log << "Failed to load font file " << filename << ": " << error << endl;
    return Face();
  }

  // Add to map and return
  faces[filename] = face;
  return Face(face);
}

// Destructor - free the faces, then the library
Cache::~Cache()
{
  for(auto p: faces) FT_Done_Face(p.second);

  if (library) FT_Done_FreeType(library);
}

}} // namespaces
