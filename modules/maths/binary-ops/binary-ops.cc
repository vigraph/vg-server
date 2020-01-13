//==========================================================================
// Maths binary function multi-module
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../maths-module.h"
#include <cmath>
#include <cfloat>

namespace {

//==========================================================================
// Binary function template - parameter is simple binary function (e.g. mod)
typedef double (*binary_op)(double x, double y);

template <binary_op op>
class BinaryFunction: public SimpleElement
{
private:
  // Clone
  BinaryFunction<op> *create_clone() const override
  {
    return new BinaryFunction<op>{module};
  }

  // Tick
  void tick(const TickData& td) override
  {
    const auto nsamples = td.samples_in_tick(output.get_sample_rate());
    sample_iterate(td, nsamples, {}, tie(x, y), tie(output),
                   [&](double x, double y, double & o)
                   {
                     o = op(x, y);
                   });
  }

public:
  using SimpleElement::SimpleElement;

  Input<double> x;
  Input<double> y;
  Output<double> output;
};

#define DECLARE_BINARY(_uc_name, _lc_name, _x_name, _y_name) \
using _uc_name##Function = BinaryFunction<_lc_name>; \
Dataflow::SimpleModule _lc_name##_module             \
{                                                    \
  #_lc_name,                                         \
  #_uc_name,                                         \
  "maths",                                           \
  {},                                                \
  {                                                  \
    { _x_name, &_uc_name##Function::x },            \
    { _y_name, &_uc_name##Function::y }             \
  },                                                 \
  {                                                  \
    { "output", &_uc_name##Function::output }        \
  }                                                  \
};                                                   \
VIGRAPH_MODULE_NEW_FACTORY(_uc_name##Function, _lc_name##_module)

inline double mod(double x, double y) { return y? fmod(x, y) : 0; }
DECLARE_BINARY(Mod, mod, "input", "modulus")
inline double power(double x, double y) { return pow(x, y); }
DECLARE_BINARY(Power, power, "input", "exponent")
inline double add(double x, double y) { return x+y; }
DECLARE_BINARY(Add, add, "input", "offset")
inline double subtract(double x, double y) { return x-y; }
DECLARE_BINARY(Subtract, subtract, "input", "offset")
inline double multiply(double x, double y) { return x*y; }
DECLARE_BINARY(Multiply, multiply, "input", "factor")
inline double divide(double x, double y)
{ return y ? (x/y) : (x<0?DBL_MIN:DBL_MAX); }
DECLARE_BINARY(Divide, divide, "input", "factor")

} // anon

#define REGISTER_BINARY(_uc_name, _lc_name)           \
  VIGRAPH_MODULE_INIT_REGISTER(_uc_name##Function, _lc_name##_module)

VIGRAPH_MODULE_INIT_START
  REGISTER_BINARY(Mod, mod)
  REGISTER_BINARY(Power, power)
  REGISTER_BINARY(Add, add)
  REGISTER_BINARY(Subtract, subtract)
  REGISTER_BINARY(Multiply, multiply)
  REGISTER_BINARY(Divide, divide)
VIGRAPH_MODULE_INIT_END

