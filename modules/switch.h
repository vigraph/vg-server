//==========================================================================
// ViGraph dataflow modules: switch.h
//
// Switch template
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_MODULE_SWITCH_H
#define __VIGRAPH_MODULE_SWITCH_H

#include "ot-log.h"
#include "vg-dataflow.h"

using namespace std;
using namespace ObTools;
using namespace ViGraph;
using namespace ViGraph::Dataflow;

template<typename T> void fade(T& value, double factor);

//==========================================================================
// Switch class template
template<typename T>
class Switch: public DynamicElement
{
private:
  void setup() override
  {
    const auto ninputs = static_cast<unsigned>(max(inputs.get(), 2));
    while (input_list.size() > ninputs)
    {
      const auto i = input_list.size();
      module.erase_input("input" + Text::itos(i));
      input_list.pop_back();
    }
    while (input_list.size() < ninputs)
    {
      input_list.emplace_back(new Input<T>{});
      const auto i = input_list.size();
      module.add_input("input" + Text::itos(i), input_list.back().get());
    }
    if (active >= static_cast<int>(input_list.size()))
      active = -1;
  }

  void tick(const TickData& td) override
  {
    const auto sample_rate = output.get_sample_rate();
//    const auto sample_duration = td.sample_duration(sample_rate);
    const auto nsamples = td.samples_in_tick(sample_rate);

    auto out = output.get_buffer();

    map<int, const vector<T> *> input_buffers;
    if (active >= 0)
      input_buffers[active] = &input_list[active]->get_buffer();

    for (auto i = 0u; i < nsamples; ++i)
    {
      auto n = active;
      if (number.connected())
      {
        n = min(max(static_cast<int>(number) - 1, -1),
                static_cast<int>(input_list.size()) - 1);
      }
      else if (fraction.connected())
      {
        n = min(max(static_cast<int>(fraction * input_list.size()), 0),
                static_cast<int>(input_list.size()) - 1);
      }
      else if (next.connected())
      {
        if (++active >= static_cast<int>(input_list.size()))
          active = 0;
      }
      if (active != n)
      {
        active = n;
        if (active >= 0)
          input_buffers[active] = &input_list[active]->get_buffer();
      }

      auto v = (active >= 0)
                ? (i <= input_buffers[active]->size()
                   ? (*input_buffers[active])[i] : T{})
                : T{};
      out.data.emplace_back(v);
    }
  }

  vector<shared_ptr<Input<T>>> input_list;
  struct State
  {
    enum class Fade
    {
      in,
      none,
      out,
    } fade;
    double factor = 0;
  };
  map<int, State> states;
  int active = -1;

public:
  using DynamicElement::DynamicElement;

  Setting<int> inputs{2};

  Input<double> number{0};
  Input<double> fraction{0};
  Input<double> next{0};
  Input<double> fade_in_time{0};
  Input<double> fade_out_time{0};

  Output<T> output;
};

#endif // !__VIGRAPH_MODULE_SWITCH_H
