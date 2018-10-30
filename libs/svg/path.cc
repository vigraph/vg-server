//==========================================================================
// ViGraph SVG library: path.cc
//
// Implementation of SVG <path>
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-svg.h"
#include "ot-lex.h"
#include "ot-text.h"
#include "ot-log.h"
#include <sstream>

namespace ViGraph { namespace SVG {

using namespace ObTools;

// Read from string
void Path::read(const string& d, Colour::RGB colour)
{
  istringstream iss(d);
  Lex::Analyser lex(iss);
  lex.add_symbol(",");
  lex.disallow_alphanum_names();  // M0 => M 0

  try
  {
    Lex::Token token = lex.read_token();
    bool is_start = true;
    while (token.type != Lex::Token::END)
    {
      if (token.type != Lex::Token::NAME || token.value.size() != 1)
        throw runtime_error("Unexpected token: "+token.value);

      char c = token.value[0];
      bool absolute = (c <= 'Z');
      Command::Type type;
      int n;  // number of values to read
      switch (toupper(c))
      {
        case 'M':
          // Open a new segment
          segments.push_back(Segment(colour));
          type = Command::Type::moveto;
          n = 2;
          break;

        case 'Z': type = Command::Type::closepath;         n = 0; break;
        case 'L': type = Command::Type::lineto;            n = 2; break;
        case 'H': type = Command::Type::horizontal_lineto; n = 1; break;
        case 'V': type = Command::Type::vertical_lineto;   n = 1; break;
        case 'C': type = Command::Type::curveto;           n = 6; break;
        case 'S': type = Command::Type::smooth_curveto;    n = 4; break;
        case 'Q':
          type = Command::Type::quadratic_bezier_curveto; n = 4; break;
        case 'T':
          type = Command::Type::smooth_quadratic_bezier_curveto; n = 2; break;
        case 'A': type = Command::Type::elliptical_arc;    n = 7; break;

        default: throw runtime_error("Unknown command: "+token.value);
      }

      if (segments.empty()) throw runtime_error("No Moveto at start");
      Segment& seg = segments.back();

      do  // loop on repeated commands
      {
        // Moveto is always absolute if first in the path, but linetos in
        // subsequent repeats keep original m/M state.  This is only true
        // for the first m in the path.  On Wednesdays with a full moon.
        seg.commands.push_back(Command(type,
            (type==Command::Type::moveto && is_start)?true:absolute));
        Command& cmd = seg.commands.back();

        // Read values
        for(int i=0; i<n; i++)
        {
          token = lex.read_token();
          // Allow , only after first
          if (i && token.type == Lex::Token::SYMBOL)
            token = lex.read_token();
          if (token.type != Lex::Token::NUMBER)
            throw runtime_error("Bad value "+token.value);

          coord_t v = Text::stof(token.value);
          cmd.values.push_back(v);
        }

        // If it was a moveto, switch to lineto for repeats (SVG/paths 8.3.2)
        if (type == Command::Type::moveto)
          type = Command::Type::lineto;

        token = lex.read_token();

        // Skip optional comma between repeats
        if (token.type == Lex::Token::SYMBOL)
          token = lex.read_token();

        // If it's a number, put back for next time
        if (token.type == Lex::Token::NUMBER)
          lex.put_back(token);

      } while (token.type == Lex::Token::NUMBER);  // repeat with no letter

      is_start = false;
    }
  }
  catch (Lex::Exception e)
  {
    throw runtime_error(e.error);
  }
}

// Read from all SVG <paths> in the given document
// Throws runtime_error on any error
void Path::read(const XML::Element& xml)
{
  const auto paths = xml.get_descendants("path");
  for(const auto p: paths)
  {
    // Get colour from style attribute ...;stroke:#rrggbb;...
    Colour::RGB colour = Colour::white;
    const string& style = (*p)["style"];
    if (!style.empty())
    {
      const vector<string>& props = Text::split(style, ';');
      for(const auto& prop: props)
      {
        const vector<string>& bits = Text::split(prop, ':');
        if (bits.size() == 2 && bits[0] == "stroke")
        {
          try
          {
            colour = Colour::RGB(bits[1]);
          }
          catch (exception) {}  // Can fail on e.g. "none"
        }
      }
    }

    const string& path_data = (*p)["d"];
    read(path_data, colour);
  }
}

// Render to points and normalise
void Path::render(vector<Point>& points, double precision, bool normalise)
{
  Point p;        // Last point - note has to span segments because movetos
                  // can be relative after first one in a path
  for(const auto& seg: segments)
  {
    auto c = seg.colour;
    Point start;    // Start of this segment
    Point control;  // Last Bezier control point
    for(const auto& cmd: seg.commands)
    {
      const auto& v = cmd.values;
      Vector offset;
      if (!cmd.is_absolute) offset = p;

      switch (cmd.type)
      {
        case Command::Type::moveto:
          start = control = p = Point(v[0], v[1])+offset;
          points.push_back(p);
          p.c = c;  // Set for when used by subsequent curves
          break;

        case Command::Type::closepath:
          control = p = start;
          p.c = c;  // It's a line back to start
          points.push_back(p);
          break;

        case Command::Type::lineto:
          control = p = Point(v[0], v[1])+offset;
          p.c = c;
          points.push_back(p);
          break;

        case Command::Type::horizontal_lineto:
          control = p = Point(v[0]+offset.x, p.y, c);  // y from last point
          points.push_back(p);
          break;

        case Command::Type::vertical_lineto:
          control = p = Point(p.x, v[0]+offset.y, c);  // x from last point
          points.push_back(p);
          break;

        case Command::Type::curveto:
          p = render_curveto(v, p, c, offset, control, points, precision);
          break;

        case Command::Type::smooth_curveto:
          p = render_smooth_curveto(v, p, c, offset, control, points, precision);
          break;

        case Command::Type::quadratic_bezier_curveto:
          p = render_quadratic(v, p, c, offset, control, points, precision);
          break;

        case Command::Type::smooth_quadratic_bezier_curveto:
          p = render_quadratic(v, p, c, offset, control, points, precision);
          break;

        case Command::Type::elliptical_arc:
          control = p = render_elliptical_arc(v, p, c, offset, points, precision);
          break;
      }
    }
  }

  // Normalise to (-0.5..0.5)
  if (normalise && !points.empty())
  {
    Point min(points[0]);
    Point max(points[0]);

    for(const auto& p: points)
    {
      if (p.x < min.x) min.x = p.x;
      if (p.y < min.y) min.y = p.y;
      if (p.x > max.x) max.x = p.x;
      if (p.y > max.y) max.y = p.y;
    }

    // Shift centre to 0,0
    Point centre = (max+min)/2;
    for(auto& p: points) p-=centre;

    Point span = max-min;
    coord_t largest = (span.x > span.y)?span.x:span.y;

    // Scale and invert
    if (largest > 0)
    {
      for(auto& p: points)
      {
        p/=largest;
        p.y = -p.y;
      }
    }
  }
}

// Render a curveto - returns last point and updates control
Point Path::render_curveto(const vector<coord_t>& v, const Point& p,
                           Colour::RGB c,
                           const Vector& offset, Point& control,
                           vector<Point>& points, double precision)
{
  Point last(v[4], v[5], c);    // last point for next time
  last += offset;
  control = Point(v[2], v[3])+offset;  // keep for next smooth
  CubicBezier b(p, Point(v[0], v[1])+offset, control, last);

  // Note, start at one step in, to not duplicate the start point
  for(double t=precision; t<=1.0; t+=precision)
    points.push_back(b.interpolate(t));
  return last;
}

// Render a smooth curveto - returns last point and updates control
Point Path::render_smooth_curveto(const vector<coord_t>& v, const Point& p,
                                  Colour::RGB c, const Vector& offset,
                                  Point& control,
                                  vector<Point>& points, double precision)
{
  Point last(v[2], v[3], c);  // last point for next time
  last += offset;
  Point reflected_control = p+(p-control);
  control = Point(v[0], v[1])+offset;  // keep for next smooth
  CubicBezier b(p, reflected_control, control, last);
  for(double t=precision; t<=1.0; t+=precision)
    points.push_back(b.interpolate(t));
  return last;
}

// Render a quadratic bezier - updates control for next time
Point Path::render_quadratic(const vector<coord_t>& v, const Point& p,
                             Colour::RGB c, const Vector& offset,Point& control,
                             vector<Point>& points, double precision)
{
  Point last(v[2], v[3], c);  // last point for next time
  last += offset;
  control = Point(v[0], v[1])+offset;  // keep for next smooth
  QuadraticBezier b(p, control, last);
  for(double t=precision; t<=1.0; t+=precision)
    points.push_back(b.interpolate(t));
  return last;
}

// Render a smooth quadratic bezier - uses previous control and updates
Point Path::render_smooth_quadratic(const vector<coord_t>& v, const Point& p,
                                    Colour::RGB c, const Vector& offset,
                                    Point& control,
                                    vector<Point>& points, double precision)
{
  Point last(v[0], v[1], c);  // last point for next time
  last += offset;
  control = p+(p-control);
  QuadraticBezier b(p, control, last);
  for(double t=precision; t<=1.0; t+=precision)
    points.push_back(b.interpolate(t));
  return last;
}

// Render an elliptical arc
Point Path::render_elliptical_arc(const vector<coord_t>& /*v*/,
                                  const Point& p,
                                  Colour::RGB /*c*/,
                                  const Vector& offset,
                                  vector<Point>& /*points*/, double /*precision*/)
{
  // !!!
  return p+offset;
}

}} // namespaces
