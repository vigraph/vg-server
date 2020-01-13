//==========================================================================
// Gate template
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_MODULE_GATE_H
#define __VIGRAPH_MODULE_GATE_H

#include "ot-log.h"
#include "vg-dataflow.h"

using namespace std;
using namespace ObTools;
using namespace ViGraph;
using namespace ViGraph::Dataflow;

//==========================================================================
// Gate class template
template<typename T>
class Gate: public SimpleElement
{
private:
  bool is_open = false;

  void tick(const TickData& td) override
  {
    const auto sample_rate = output.get_sample_rate();
    const auto nsamples = td.samples_in_tick(sample_rate);

    if (control.connected())
    {
      sample_iterate(td, nsamples, {}, tie(input, control),
                     tie(output),
                     [&](T i, Number c, T& o)
      {
        is_open = c;
        if (is_open)
          o = i;
        else
          o = T{};
      });
    }
    else
    {
      sample_iterate(td, nsamples, {}, tie(input, open, close),
                     tie(output),
                     [&](T i, Trigger op, Trigger cl, T& o)
      {
        if (is_open)
        {
          if (cl)
            is_open = false;
          if (op)
            is_open = true;
        }
        else
        {
          if (op)
            is_open = true;
          if (cl)
            is_open = false;
        }
        if (is_open)
          o = i;
        else
          o = T{};
      });
    }
  }

public:
  using SimpleElement::SimpleElement;

  Input<T> input{};
  Input<Number> control{0};
  Input<Trigger> open{0};
  Input<Trigger> close{0};

  Output<T> output;
};

#endif // !__VIGRAPH_MODULE_GATE_H
