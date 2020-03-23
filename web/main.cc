//==========================================================================
// Main file for ViGraph web assembly
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-dataflow.h"

using namespace std;
using namespace ObTools;
using namespace ViGraph;

namespace {
const auto tick_interval = Time::Duration{0.04};
}


int main(int, char **)
{
  Dataflow::Engine engine{};
  engine.set_tick_interval(tick_interval);
  auto next_tick = Time::Duration::clock();
  while (true)
  {
    auto now = Time::Duration::clock();
    engine.tick(now);
    now = Time::Duration::clock();
    next_tick += tick_interval;
    if (now < next_tick)
      this_thread::sleep_for(
          chrono::duration<double>((next_tick - now).seconds()));
  }
  return 0;
}
