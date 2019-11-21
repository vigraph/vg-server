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

template<typename T> inline T switch_fade(const T& value, double factor);

//==========================================================================
// Switch class template
template<typename T>
class Switch: public DynamicElement
{
private:
  vector<shared_ptr<Input<T>>> input_list;
  struct State
  {
    enum class Fade
    {
      in,
      full,
      out,
    } fade;
    double factor = 0;
  };
  map<int, State> states;
  int active = -1;

  void setup(const SetupContext&) override
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
    const auto sample_duration = td.sample_duration(sample_rate);
    const auto fade_in_inc = fade_in_time ? sample_duration / fade_in_time
                                          : 1.0;
    const auto fade_out_dec = fade_out_time ? sample_duration / fade_out_time
                                            : 1.0;
    const auto nsamples = td.samples_in_tick(sample_rate);

    auto out = output.get_buffer();

    struct InputData
    {
      const vector<T> *data = nullptr;
      State *state = nullptr;
      InputData(const vector<T> *_data, State *_state):
        data{_data}, state{_state}
      {}
    };
    map<int, InputData> id;
    for (const auto& p: states)
    {
      id.emplace(p.first,
                 InputData{&input_list[p.first]->get_buffer(),
                           &states[p.first]});
    }

    enum class SwitchOn
    {
      none,
      number,
      fraction,
      next
    };
    auto switch_on = SwitchOn::none;
    auto switch_input = static_cast<const vector<double> *>(nullptr);
    auto switch_last = 0.0;
    if (number.connected())
    {
      switch_on = SwitchOn::number;
      switch_input = &number.get_buffer();
      switch_last = number.get();
    }
    else if (fraction.connected())
    {
      switch_on = SwitchOn::fraction;
      switch_input = &fraction.get_buffer();
      switch_last = fraction.get();
    }
    else if (next.connected())
    {
      switch_on = SwitchOn::next;
      switch_input = &next.get_buffer();
      switch_last = 0;
    }

    for (auto i = 0u; i < nsamples; ++i)
    {
      auto n = active;
      const auto inp = switch_input
                       ? (i < switch_input->size()
                          ? (*switch_input)[i]
                          : (switch_input->empty()
                             ? switch_last
                             : switch_input->back()
                            )
                         )
                       : 0.0;
      switch (switch_on)
      {
        case SwitchOn::number:
          n = min(max(static_cast<int>(inp) - 1, -1),
                  static_cast<int>(input_list.size()) - 1);
          break;
        case SwitchOn::fraction:
          n = min(max(static_cast<int>(inp * input_list.size()), 0),
                  static_cast<int>(input_list.size()) - 1);
          break;
        case SwitchOn::next:
          if (inp)
            if (++n >= static_cast<int>(input_list.size()))
              n = 0;
          break;
        case SwitchOn::none:
            break;
      }
      if (active != n)
      {
        if (active >= 0)
          states[active].fade = State::Fade::out;
        active = n;
        if (active >= 0)
        {
          states[active].fade = State::Fade::in;
          if (id.find(active) == id.end())
            id.emplace(active,
                       InputData{&input_list[active]->get_buffer(),
                                 &states[active]});
        }
      }

      auto v = T{};
      for (auto idit = id.begin(); idit != id.end();)
      {
        auto n = idit->first;
        auto& b = *idit->second.data;
        auto& s = *idit->second.state;
        auto removed = false;
        switch (s.fade)
        {
          case State::Fade::in:
            s.factor += fade_in_inc;
            if (s.factor >= 1)
            {
              s.factor = 1;
              s.fade = State::Fade::full;
            }
            if (i < b.size())
              v += switch_fade(b[i], s.factor);
            break;

          case State::Fade::full:
            if (i < b.size())
              v += b[i];
            break;

          case State::Fade::out:
            {
              auto complete = false;
              s.factor -= fade_out_dec;
              if (s.factor <= 0)
              {
                s.factor = 0;
                complete = true;
              }
              if (i < b.size())
                v += switch_fade(b[i], s.factor);
              if (complete)
              {
                states.erase(n);
                idit = id.erase(idit);
                removed = true;
              }
            }
            break;
        }
        if (!removed)
          ++idit;
      }
      out.data.emplace_back(v);
    }
  }

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
