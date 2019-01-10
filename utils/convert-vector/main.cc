//==========================================================================
// ViGraph vector graphics: convert-vector.cc
//
// Utility to convert vector files
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-ilda.h"
#include "vg-svg.h"
#include "vg-laser.h"
#include "ot-log.h"
#include "ot-text.h"
#include <fstream>

using namespace std;
using namespace ObTools;
using namespace ViGraph;
using namespace ViGraph::Geometry;
using namespace ViGraph::Laser;

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  if (argc < 2)
  {
    cout << "ViGraph vector file convertor v0.1\n\n";
    cout << "Usage:\n";
    cout << "  " << argv[0] << "[options] <input file> <output file>\n\n";
    cout << "Options:\n";
    cout << "  -p --precision <n>   Interpolation precision (default 0.1)\n";
    cout << "  -i --infill <n>      Infill points to maintain maximum distance\n";
    cout << "  -v --vertex <a> <n>  Add <n> points at vertices over angle <a> (deg)\n";
    cout << endl;
    cout << "Currently only converts SVG to ILDA\n";
    return 2;
  }

  double precision = SVG::Path::default_precision;
  double max_distance = 0;
  double max_angle = -1;
  int vertex_repeats = 0;

  for(int i=1; i<argc-2; i++)
  {
    string arg(argv[i]);
    if ((arg == "-p" || arg == "--precision") && ++i < argc-2)
      precision = Text::stof(argv[i]);
    else if ((arg == "-i" || arg == "--infill") && ++i < argc-2)
      max_distance = Text::stof(argv[i]);
    else if ((arg == "-v" || arg == "--vertex") && ++i < argc-3)
    {
      max_angle = Text::stof(argv[i])*pi/180;  // deg->rad
      vertex_repeats = Text::stoi(argv[++i]);
    }
    else
    {
      cerr << "Unknown option: " << arg << endl;
      return 2;
    }
  }

  const string svgf(argv[argc-2]);
  cout << "Reading SVG file " << svgf << endl;
  XML::Configuration cfg(svgf, cerr);
  if (!cfg.read())
  {
    cerr << "Can't read SVG file " << svgf << endl;
    return 2;
  }

  // Get all paths from XML
  XML::Element& root = cfg.get_root();
  SVG::Path path;
  path.read(root);

  // Create the ILDA 'animation' (single frame)
  ILDA::Animation animation;
  ILDA::Frame frame(ILDA::Frame::Format::true_2d);

  // Render to points
  path.render(frame.points, precision, true);  // with normalisation
  cout << "Read " << frame.points.size() << " points in frame\n";

  // Optimise if requested
  if (max_distance > 0 || max_angle >= 0)
  {
    Laser::Optimiser optimiser;
    if (max_distance > 0)
      frame.points = optimiser.infill_lines(frame.points, max_distance);

    if (max_angle >= 0)
      frame.points = optimiser.add_vertex_repeats(frame.points, max_angle,
                                                  vertex_repeats);
    cout << "After optimisation, " << frame.points.size()
         << " points in frame\n";
  }

  // Construct single-frame animation
  animation.frames.push_back(frame);

  // Write out
  const string ildf(argv[argc-1]);
  cout << "Writing ILDA file " << ildf << endl;
  ofstream outfile(ildf);
  if (!outfile)
  {
    cerr << "Can't write file " << ildf << endl;
    return 2;
  }

  ILDA::Writer writer(outfile);
  writer.write(animation);
  outfile.close();

  return 0;
}




