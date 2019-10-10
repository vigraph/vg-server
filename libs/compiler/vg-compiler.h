//==========================================================================
// ViGraph vg-to-json compiler: vg-compiler.h
//
// Definition of compiler structures and operations
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VG_COMPILER_H
#define __VG_COMPILER_H

#include <string>
#include <vector>
#include "ot-lex.h"
#include "ot-json.h"

namespace ViGraph { namespace Compiler {

using namespace std;
using namespace ObTools;

//==========================================================================
// Parser exception
struct Exception
{
  string error;
  Exception(const string& _error): error(_error) {}
};

//==========================================================================
// Parser
class Parser
{
  Lex::Analyser lex;
  map<string, int> type_serials;
  string default_section;

  // Internals
  void read_inputs(JSON::Value& element);
  void read_outputs(JSON::Value& element);
  void sanity_check(const JSON::Value& root);

public:
  //------------------------------------------------------------------------
  // Constructor on an istream
  Parser(istream& _input);

  //------------------------------------------------------------------------
  // Set default section name
  void set_default_section(const string& s) { default_section = s; }

  //------------------------------------------------------------------------
  // Read structure as JSON
  JSON::Value get_json();
};

//==========================================================================
}} //namespaces
#endif // !__VG_COMPILER_H
