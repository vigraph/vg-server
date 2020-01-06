//==========================================================================
// ViGraph dataflow module: core/function/func.h
//
// Type for function selection
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../core-module.h"

//--------------------------------------------------------------------------
// Function type
enum class Func
{
  none,

  // unary
  sin,
  cos,
  tan,
  asin,
  acos,
  atan,
  log10,
  log,
  exp10,
  exp,
  sqrt,
  square,
  cube,
  inverse,
  floor,
  ceil,
  round,
  abs,

  // binary
  mod,
  power
};

namespace ViGraph { namespace Dataflow {

template<> inline
string get_module_type<Func>() { return "function"; }

template<> inline void set_from_json(Func& func,
                                     const JSON::Value& json)
{
  const auto& f = json.as_str();

  if (f == "none")
    func = Func::none;
  else if (f == "sin")
    func = Func::sin;
  else if (f == "cos")
    func = Func::cos;
  else if (f == "tan")
    func = Func::tan;
  else if (f == "asin")
    func = Func::asin;
  else if (f == "acos")
    func = Func::acos;
  else if (f == "atan")
    func = Func::atan;
  else if (f == "log10")
    func = Func::log10;
  else if (f == "log")
    func = Func::log;
  else if (f == "exp10")
    func = Func::exp10;
  else if (f == "exp")
    func = Func::exp;
  else if (f == "sqrt")
    func = Func::sqrt;
  else if (f == "square")
    func = Func::square;
  else if (f == "cube")
    func = Func::cube;
  else if (f == "inverse")
    func = Func::inverse;
  else if (f == "floor")
    func = Func::floor;
  else if (f == "ceil")
    func = Func::ceil;
  else if (f == "round")
    func = Func::round;
  else if (f == "abs")
    func = Func::abs;
  else if (f == "mod")
    func = Func::mod;
  else if (f == "power")
    func = Func::power;
  else
    throw runtime_error("No such function "+f);
}

template<> inline JSON::Value get_as_json(const Func& func)
{
  switch (func)
  {
    case Func::none:
      return "none";
    case Func::sin:
      return "sin";
    case Func::cos:
      return "cos";
    case Func::tan:
      return "tan";
    case Func::asin:
      return "asin";
    case Func::acos:
      return "acos";
    case Func::atan:
      return "atan";
    case Func::log10:
      return "log10";
    case Func::log:
      return "log";
    case Func::exp10:
      return "exp10";
    case Func::exp:
      return "exp";
    case Func::sqrt:
      return "sqrt";
    case Func::square:
      return "square";
    case Func::cube:
      return "cube";
    case Func::inverse:
      return "inverse";
    case Func::floor:
      return "floor";
    case Func::ceil:
      return "ceil";
    case Func::round:
      return "round";
    case Func::abs:
      return "abs";
    case Func::mod:
      return "mod";
    case Func::power:
      return "power";
  }
}

}} // namespaces

