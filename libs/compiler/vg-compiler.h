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

  // Internals
  void read_inputs(JSON::Value& element);
  void read_outputs(JSON::Value& element);

public:
  //------------------------------------------------------------------------
  // Constructor on an istream
  Parser(istream& _input);

  //------------------------------------------------------------------------
  // Read structure as JSON
  JSON::Value get_json();
};

//==========================================================================
}} //namespaces
#endif // !__VG_COMPILER_H
