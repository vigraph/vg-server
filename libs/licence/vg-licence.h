//==========================================================================
// ViGraph vector graphics: vg-licence.h
//
// Definitions for licence management library
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VG_LICENCE_H
#define __VG_LICENCE_H

#include "ot-xml.h"
#include "ot-file.h"

namespace ViGraph { namespace Licence {

// Make our lives easier without polluting anyone else
using namespace std;
using namespace ObTools;

//==========================================================================
// Licence file
class File: public XML::Configuration
{
public:
  //------------------------------------------------------------------------
  // Constructor
  // _serr gives stream for errors
  File(const string& fn, ostream& _serr = cerr);

  //------------------------------------------------------------------------
  // Verify validity of licence - checks signature and window
  // Independent of useability on current machine
  bool validate();

  //------------------------------------------------------------------------
  // Check usability of licence file on current machine
  // Also validates signature and window
  bool check();

  //------------------------------------------------------------------------
  // Get string for given description element
  string get_description(const string& name);
  string operator[](const string& name) { return get_description(name); }

  //------------------------------------------------------------------------
  // Get XML element for given named component (e.g. ps:pumpd)
  // Pointer to element, or 0 if not found.  Element is owned by File class
  // and will be destroyed by it
  XML::Element *get_component(const string& name);

  //------------------------------------------------------------------------
  // Sign an existing licence with private key (PEM format)
  // Writes modified XML back to file
  // Returns whether successful
  bool sign(const string& private_key, const string& pass_phrase="");
};

//==========================================================================
}} //namespaces
#endif // !__VG_LICENCE_H



