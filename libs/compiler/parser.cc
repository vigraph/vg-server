//==========================================================================
// ViGraph vg-to-json compiler: parser.cc
//
// Implementation of VG format parser
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-compiler.h"
#include <sstream>
#include <algorithm>

namespace ViGraph { namespace Compiler {

//--------------------------------------------------------------------------
// Constructor on an istream
Parser::Parser(istream& input): lex(input)
{
  // Add symbols
  lex.add_symbol("=");
  lex.add_symbol("->");
  lex.add_symbol(".");
  lex.add_symbol(">");
  lex.add_symbol(":");
}

//------------------------------------------------------------------------
// Read structure as JSON
JSON::Value Parser::get_json()
{
  try
  {
    return JSON::Value();
  }
  catch (const Lex::Exception& e)
  {
    throw Exception(e.error);
  }
}


}} // namespaces
