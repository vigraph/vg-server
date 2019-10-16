//==========================================================================
// ViGraph vector graphics: main.cc
//
// Utility to compile 'vg' text format to JSON graph
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-compiler.h"
#include <fstream>

using namespace std;
using namespace ObTools;
using namespace ViGraph;

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  if (argc < 2)
  {
    cout << "ViGraph 'vg' compiler v0.1\n\n";
    cout << "Usage:\n";
    cout << "  " << argv[0] << " [options] <input file> <output file>\n\n";
    cout << "Options:\n";
    cout << "  -d --default-section <s>   Set default section (default 'core')\n";
    cout << endl;
    cout << "Both <input file> and <output file> can be '-' for pipeline\n";
    return 2;
  }

  string default_section = "core";
  for(int i=1; i<argc-2; i++)
  {
    string arg(argv[i]);
    if ((arg == "-d" || arg == "--default-section") && ++i < argc-2)
      default_section = argv[i];
    else
    {
      cerr << "Unknown option: " << arg << endl;
      return 2;
    }
  }

  JSON::Value root;

  try
  {
    const string vgf(argv[argc-2]);
    if (vgf == "-")
    {
      Compiler::Parser parser(cin);
      parser.set_default_section(default_section);
      root = parser.get_elements_json();
    }
    else
    {
      ifstream in(vgf);
      if (!in)
      {
        cerr << "Can't read 'vg' file " << vgf << endl;
        return 2;
      }

      Compiler::Parser parser(in);
      parser.set_default_section(default_section);
      root = parser.get_elements_json();
    }
  }
  catch (Compiler::Exception e)
  {
    cerr << "Compile failed:\n" << e.error << endl;
    return 4;
  }

  // Write out
  const string jsf(argv[argc-1]);
  if (jsf == "-")
  {
    cout << root;
  }
  else
  {
    cout << "Writing JSON file " << jsf << endl;
    ofstream out(jsf);
    if (!out)
    {
      cerr << "Can't write file " << jsf << endl;
      return 2;
    }

    out << root;
  }

  return 0;
}




