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
  lex.add_symbol(">");
  lex.add_symbol(".");
  lex.add_symbol("-");
  lex.add_symbol(":");
  lex.add_line_comment_symbol("#");
}

//------------------------------------------------------------------------
// Try to read inputs name=value onto the given element
// Stops and pushes back if not 'name =' format
void Parser::read_inputs(JSON::Value& element)
{
  for(;;) // Looping on inputs
  {
    // Read ahead another name
    auto token = lex.read_token();
    if (token.type != Lex::Token::NAME)
    {
      lex.put_back(token);
      break;
    }

    // Then optional =
    auto symbol = lex.read_token();
    if (symbol.type == Lex::Token::SYMBOL && symbol.value == "=")
    {
      // It's an input assignment

      // Create inputs if none yet
      if (!element["inputs"])
        element.put("inputs", JSON::Value::OBJECT);
      auto& inputs = element["inputs"];
      auto& input = inputs.put(token.value, JSON::Value::OBJECT);

      auto value = lex.read_token();
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
      lex.put_back(token);
      break;
    }
  }
}

//------------------------------------------------------------------------
// Try to read outputs [<output>]>[<id>.]<input> onto the given element
// Stops and pushes back if not > format
void Parser::read_outputs(JSON::Value& element)
{
  for(;;) // Looping on outputs
  {
    // Read ahead another name
    auto token = lex.read_token();

    // Special case for '-' = 'output'
    if (token.type == Lex::Token::SYMBOL && token.value == "-")
      token = Lex::Token(Lex::Token::NAME, "output");

    if (token.type != Lex::Token::NAME)
    {
      lex.put_back(token);
      break;
    }

    // Then optional ->
    auto symbol = lex.read_token();
    if (symbol.type == Lex::Token::SYMBOL && symbol.value == ">")
    {
      // It's an output assignment

      // Create outputs if none yet
      if (!element["outputs"])
        element.set("outputs", JSON::Value::OBJECT);
      auto& outputs = element["outputs"];

      // Create this output if not already done
      if (!outputs[token.value])
        outputs.put(token.value, JSON::Value::OBJECT)
               .put("connections", JSON::Value::ARRAY);

      auto& output = outputs[token.value];
      auto& conns = output["connections"];
      auto& conn = conns.add(JSON::Value::OBJECT);

      // Try to read <element> .
      auto el = lex.read_token();
      auto dot = lex.read_token();
      if (el.type == Lex::Token::NAME
          && dot.type == Lex::Token::SYMBOL && dot.value == ".")
      {
        conn.put("element", el.value);
      }
      else
      {
        // Revert
        lex.put_back(dot);
        lex.put_back(el);
      }

      auto input = lex.read_token();

      // Special case for '-' = 'input'
      if (input.type == Lex::Token::SYMBOL && input.value == "-")
        input = Lex::Token(Lex::Token::NAME, "input");

      if (input.type != Lex::Token::NAME)
        throw Exception("Unexpected token in output route "+input.value);

      conn.put("input", input.value);
    }
    else
    {
      // Put both back and go to next element
      lex.put_back(symbol);
      lex.put_back(token);
      break;
    }
  }
}

//------------------------------------------------------------------------
// Final sanity check that everything is connected
void Parser::sanity_check(const JSON::Value& root)
{
  for(auto& eit: root.o)
  {
    const auto& outputs = eit.second["outputs"];
    for(auto& oit: outputs.o)
    {
      const auto& connections = oit.second["connections"];
      for(auto& cit: connections.a)
      {
        // Fail if not set
        const auto& id = cit["element"].as_str();
        if (id.empty())
          throw Exception("Unconnected output "+oit.first
                          +" in element "+eit.first);

        // Fail if it doesn't exist
        if (!root[id])
          throw Exception("No such element "+id+" in output "+oit.first
                          +" in element "+eit.first);
      }
    }
  }
}

//------------------------------------------------------------------------
// Read structure as JSON
JSON::Value Parser::get_json()
{
  type_serials.clear();
  string last_element_id;

  try
  {
    JSON::Value root(JSON::Value::OBJECT);
    for(;;) // Looping on elements
    {
      auto token = lex.read_token();
      if (token.type == Lex::Token::END) break;

      if (token.type != Lex::Token::NAME)
        throw Exception("Expected a name");

      // Try for : to see if this is the ID prefix or the type
      string id, type;
      auto colon = lex.read_token();
      if (colon.type == Lex::Token::SYMBOL && colon.value == ":")
      {
        auto type_t = lex.read_token();
        if (type_t.type != Lex::Token::NAME)
          throw Exception("Expected a type name");
        type = type_t.value;
        id = token.value;
      }
      else
      {
        // This is the type, invent an ID
        type = token.value;
        id = type+Text::itos(++type_serials[type]);

        // Put non-colon back
        lex.put_back(colon);
      }

      // Create new element
      auto& element = root.put(id, JSON::Value::OBJECT);
      element.put("type", type);
      read_inputs(element);
      read_outputs(element);

      // Check if last element has any unconnected inputs
      if (!last_element_id.empty())
      {
        auto& last_element = root[last_element_id];
        auto& outputs = last_element["outputs"];
        for(auto& oit: outputs.o)
        {
          auto& connections = oit.second["connections"];
          for(auto& cit: connections.a)
          {
            // Set if not explicitly set already
            if (!cit["element"]) cit.put("element", id);
          }
        }
      }

      last_element_id = id;
    }

    sanity_check(root);
    return root;
  }
  catch (const Lex::Exception& e)
  {
    throw Exception(e.error);
  }
}


}} // namespaces
