//==========================================================================
// Maths unary function multi-module
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../maths-module.h"
#include <cmath>
#include <cfloat>

namespace {

//==========================================================================
// Unary function template - parameter is simple unary function (e.g. sin)
typedef double (*unary_op)(double x);

template <unary_op op>
class UnaryFunction: public SimpleElement
{
private:
  // Clone
  UnaryFunction<op> *create_clone() const override
  {
    return new UnaryFunction<op>{module};
  }

  // Tick
  void tick(const TickData& td) override
  {
    const auto nsamples = td.samples_in_tick(output.get_sample_rate());
    sample_iterate(td, nsamples, {}, tie(input), tie(output),
                   [&](double i, double & o)
                   {
                     o = op(i);
                   });
  }

public:
  using SimpleElement::SimpleElement;

  Input<double> input;
  Output<double> output;
};

#define DECLARE_UNARY(_uc_name, _lc_name)            \
using _uc_name##Function = UnaryFunction<_lc_name>;  \
Dataflow::SimpleModule _lc_name##_module             \
{                                                    \
  #_lc_name,                                         \
  #_uc_name,                                         \
  "maths",                                           \
  {},                                                \
  {                                                  \
    { "input", &_uc_name##Function::input }          \
  },                                                 \
  {                                                  \
    { "output", &_uc_name##Function::output }        \
  }                                                  \
};                                                   \
VIGRAPH_MODULE_NEW_FACTORY(_uc_name##Function, _lc_name##_module)

DECLARE_UNARY(Sin, sin)
DECLARE_UNARY(Cos, cos)
DECLARE_UNARY(Tan, tan)
DECLARE_UNARY(ASin, asin)
DECLARE_UNARY(ACos, acos)
DECLARE_UNARY(ATan, atan)
DECLARE_UNARY(Log10, log10)
DECLARE_UNARY(Log, log)
DECLARE_UNARY(Exp10, exp10)
DECLARE_UNARY(Exp, exp)
DECLARE_UNARY(Sqrt, sqrt)
inline double square(double x) { return x*x; }
DECLARE_UNARY(Square, square)
inline double cube(double x) { return x*x*x; }
DECLARE_UNARY(Cube, cube)
inline double inverse(double x) { return x? (1.0/x) : DBL_MAX; }
DECLARE_UNARY(Inverse, inverse)
DECLARE_UNARY(Floor, floor)
DECLARE_UNARY(Ceil, ceil)
DECLARE_UNARY(Round, round)
DECLARE_UNARY(Abs, abs)

} // anon

#define REGISTER_UNARY(_uc_name, _lc_name)           \
  VIGRAPH_MODULE_INIT_REGISTER(_uc_name##Function, _lc_name##_module)

VIGRAPH_MODULE_INIT_START
  REGISTER_UNARY(Sin, sin)
  REGISTER_UNARY(Cos, cos)
  REGISTER_UNARY(Tan, tan)
  REGISTER_UNARY(ASin, asin)
  REGISTER_UNARY(ACos, acos)
  REGISTER_UNARY(ATan, atan)
  REGISTER_UNARY(Log10, log10)
  REGISTER_UNARY(Log, log)
  REGISTER_UNARY(Exp10, exp10)
  REGISTER_UNARY(Exp, exp)
  REGISTER_UNARY(Sqrt, sqrt)
  REGISTER_UNARY(Square, square)
  REGISTER_UNARY(Cube, cube)
  REGISTER_UNARY(Inverse, inverse)
  REGISTER_UNARY(Floor, floor)
  REGISTER_UNARY(Ceil, ceil)
  REGISTER_UNARY(Round, round)
  REGISTER_UNARY(Abs, abs)
VIGRAPH_MODULE_INIT_END

