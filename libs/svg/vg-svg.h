//==========================================================================
// ViGraph SVG library: vg-svg.h
//
// Definition of SVG structures
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VG_SVG_H
#define __VG_SVG_H

#include "vg-geometry.h"
#include "ot-xml.h"
#include <vector>
#include <list>

namespace ViGraph { namespace SVG {

using namespace std;
using namespace ObTools;
using namespace ViGraph::Geometry;

const double default_precision{0.1};  // 10 points

// -------------------------------------------------------------------------
// Path
class Path
{
  // Internals
  Point render_curveto(const vector<coord_t>& v, const Point& p,
                       Colour::RGB c, const Vector& offset, Point& control,
                       vector<Point>& points, double precision);
  Point render_smooth_curveto(const vector<coord_t>& v, const Point& p,
                              Colour::RGB c, const Vector& offset, Point& control,
                              vector<Point>& points, double precision);
  Point render_quadratic(const vector<coord_t>& v, const Point& p,
                         Colour::RGB c, const Vector& offset, Point& control,
                         vector<Point>& points, double precision);
  Point render_smooth_quadratic(const vector<coord_t>& v, const Point& p,
                                Colour::RGB c, const Vector& offset, Point& control,
                                vector<Point>& points, double precision);
  Point render_elliptical_arc(const vector<coord_t>& v, const Point& p,
                              Colour::RGB c, const Vector& offset,
                              vector<Point>& points, double precision);

 public:
  struct Command
  {
    enum class Type
    {
      moveto,
      closepath,
      lineto,
      horizontal_lineto,
      vertical_lineto,
      curveto,
      smooth_curveto,
      quadratic_bezier_curveto,
      smooth_quadratic_bezier_curveto,
      elliptical_arc
    };
    Type type{Type::moveto};
    bool is_absolute{false};
    vector<coord_t> values;
    Command() {}
    Command(Type _t, bool _abs=false): type(_t), is_absolute(_abs) {}
  };

  struct Segment
  {
    vector<Command> commands;
    Colour::RGB colour{Colour::white};
    Segment(Colour::RGB c=Colour::white): colour(c) {}
  };

  list<Segment> segments;

  // Constructors
  Path() {}
  Path(const string& d) { read(d); }

  // Read from SVG <path d> string
  // Throws runtime_error on any error
  void read(const string& d, Colour::RGB colour=Colour::white);

  // Read from all SVG <paths> in the given document
  // Throws runtime_error on any error
  void read(const XML::Element& xml);

  // Render to points
  // precision is currently the increment of 't', hence 1/number of points
  // !!! Use adaptive fitting and make precision the tolerance
  void render(vector<Point>& points, double precision = default_precision,
              bool normalise = false);
};


//==========================================================================
}} //namespaces
#endif // !__VG_SVG_H
