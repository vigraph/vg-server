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
  lex.add_symbol("/");
  lex.add_symbol("[");
  lex.add_symbol("]");
  lex.add_line_comment_symbol("#");
  lex.allow_dashed_names();
}

//------------------------------------------------------------------------
// Try to read settings name=value onto the given element
// Stops and pushes back if not 'name =' format
void Parser::read_settings(JSON::Value& element)
{
  for(;;) // Looping on settings
  {
    // Read ahead another name or [
    auto token = lex.read_token();
    if (token.type == Lex::Token::SYMBOL && token.value == "[")
    {
      // Open sub-graph - note reads until ]
      element.put("elements", get_json());
      break;
    }
    else if (token.type != Lex::Token::NAME)
    {
      lex.put_back(token);
      break;
    }

    // Then optional =
    auto symbol = lex.read_token();
    if (symbol.type == Lex::Token::SYMBOL && symbol.value == "=")
    {
      // It's a setting assignment

      // Create settings if none yet
      if (!element["settings"])
        element.put("settings", JSON::Value::OBJECT);
      auto& settings = element["settings"];
      auto& setting = settings.put(token.value, JSON::Value::OBJECT);

      auto value = lex.read_token();
      switch (value.type)
      {
        case Lex::Token::NUMBER:
          // Is it float?
          if (value.value.find('.') != string::npos)
            setting.put("value", Text::stof(value.value));
          else
            setting.put("value", Text::stoi64(value.value));
          break;

        case Lex::Token::STRING:
          setting.put("value", value.value);
          break;

        default:
          cout << "Bad token value " << value.type << endl;
          throw Exception("Unrecognised setting value");
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
// Create subgraph inputs from connections
void Parser::create_subgraph_inputs(JSON::Value& root)
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
        auto& target = root[id];

        // Does the target have elements, so it's a subgraph?
        if (!!target && !!target["elements"])
        {
          // Create an input here
          if (!target["inputs"])
            target.set("inputs", JSON::Value::OBJECT);
          auto& inputs = target["inputs"];
          inputs.put(cit["input"].as_str(), JSON::Value::OBJECT);
        }
      }
    }
  }
}

//------------------------------------------------------------------------
// Read structure as JSON - raw object of elements
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
      if (token.type == Lex::Token::END
       || (token.type == Lex::Token::SYMBOL && token.value == "]")) break;

      if (token.type != Lex::Token::NAME)
        throw Exception("Expected a name");

      // Try for : to see if this is the ID prefix or the type
      string id, type, id_base;
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
        // This is the type, invent an ID later
        id_base = type = token.value;

        // Put non-colon back
        lex.put_back(colon);
      }

      // Try for / to see if there is a section prefix
      auto slash = lex.read_token();
      if (slash.type == Lex::Token::SYMBOL && slash.value == "/")
      {
        // The type we have is actually the section
        auto type_t = lex.read_token();
        if (type_t.type != Lex::Token::NAME)
          throw Exception("Expected a type name");
        type += '/';
        type += type_t.value;
        id_base = type_t.value;
      }
      else
      {
        // Is there a default section?
        if (!default_section.empty())
          type = default_section + '/' + type;

        // Put non-slash back
        lex.put_back(slash);
      }

      // Create an ID if none set
      if (id.empty()) id = id_base + Text::itos(++type_serials[id_base]);

      // Create new element
      auto& element = root.put(id, JSON::Value::OBJECT);
      element.put("type", type);
      read_settings(element);
      read_outputs(element);

      // Check if last element has any unconnected outputs
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
    create_subgraph_inputs(root);
    return root;
  }
  catch (const Lex::Exception& e)
  {
    throw Exception(e.error);
  }
}

//------------------------------------------------------------------------
// Read structure as JSON - object containing 'elements' object of elements
JSON::Value Parser::get_elements_json()
{
  JSON::Value root(JSON::Value::OBJECT);
  root.put("elements", get_json());
  return root;
}

}} // namespaces
