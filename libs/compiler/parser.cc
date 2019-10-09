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
  lex.add_line_comment_symbol("#");
}

//------------------------------------------------------------------------
// Try to read inputs name=value onto the given element
// Pushes back if not name = format
void Parser::read_inputs(JSON::Value& element)
{
  for(;;) // Looping on inputs
  {
    // Read ahead another name
    Lex::Token next_token = lex.read_token();
    if (next_token.type == Lex::Token::END) break;

    // Then optional =
    Lex::Token symbol = lex.read_token();
    if (symbol.type == Lex::Token::SYMBOL && symbol.value == "=")
    {
      // Create inputs if none yet
      if (!element["inputs"])
        element.set("inputs", JSON::Value::OBJECT);
      JSON::Value& inputs = element["inputs"];

      // It's an input assignment
      inputs.set(next_token.value, JSON::Value::OBJECT);
      JSON::Value& input = inputs[next_token.value];

      Lex::Token value = lex.read_token();
      switch (value.type)
      {
        case Lex::Token::NUMBER:
          // Is it float?
          if (value.value.find('.') != string::npos)
            input.put("value", Text::stof(value.value));
          else
            input.put("value", Text::stoi64(value.value));
          break;

        case Lex::Token::STRING:
          input.put("value", value.value);
          break;

        default:
          cout << "Bad token value " << value.type << endl;
          throw Exception("Unrecognised input value");
      }
    }
    else
    {
      // Put both back and go to next element
      lex.put_back(symbol);
      lex.put_back(next_token);
      break;
    }
  }
}

//------------------------------------------------------------------------
// Read structure as JSON
JSON::Value Parser::get_json()
{
  try
  {
    JSON::Value root(JSON::Value::OBJECT);
    for(;;) // Looping on elements
    {
      Lex::Token token = lex.read_token();
      if (token.type == Lex::Token::END) break;

      if (token.type != Lex::Token::NAME)
        throw Exception("Expected an element name");

      root.set(token.value, JSON::Value::OBJECT);
      JSON::Value& element = root[token.value];

      read_inputs(element);
    }
    cout << root;
    return root;
  }
  catch (const Lex::Exception& e)
  {
    throw Exception(e.error);
  }
}


}} // namespaces
