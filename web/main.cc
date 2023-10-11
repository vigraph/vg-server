//==========================================================================
// Main file for ViGraph web assembly
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-dataflow.h"
#include "vg-json.h"
#include "ot-lib.h"
#include <sstream>

using namespace std;
using namespace ObTools;
using namespace ViGraph;

namespace {
const auto tick_interval = Time::Duration{0.04};
const auto module_dir = "modules";
typedef bool vg_init_fn_t(Log::Channel&, Dataflow::Engine&);
const auto log_level = Log::Level::detail;
const auto log_time_format = "%a %d %b %H:%M:%*S [%*L]: ";
const auto log_hold_time = Time::Duration{"1 min"};
}

int main(int, char **)
{
  auto chan_out = Log::StreamChannel{&cout};
  Log::logger.connect_full(&chan_out, log_level,
                           log_time_format, log_hold_time);

  Log::Streams log;
  Dataflow::Engine engine{};
  engine.set_tick_interval(tick_interval);

  const auto dir = File::Directory{module_dir};
  if (dir.is_dir())
  {
    log.summary << "Searching directory " << dir << " for modules\n";
    list<File::Path> paths;
    dir.inspect_recursive(paths, "*.so");
    for (const auto& path: paths)
    {
      log.detail << "Loading module " << path << endl;
      auto mod = make_unique<Lib::Library>(path.str());
      if (!*mod)
      {
        log.error << "Can't open dynamic library " << path << ": "
                  << mod->get_error() << endl;
        continue;
      }

      log.detail << " - loaded\n";
      auto fn = mod->get_function<vg_init_fn_t *>("vg_init");
      if (!fn)
      {
        log.error << "No 'vg_init' symbol in dynamic library " << path << endl;
        continue;
      }

      if (!fn(Log::logger, engine))
      {
        log.error << "Module " << path << " initialisation failed\n";
        continue;
      }

      log.detail << " - initialised\n";
    }
  }

  auto iss = istringstream{R"(
{
  "elements":
  {
    "log":
    {
      "settings":
      {
        "text":
        {
          "value": "Log triggered"
        }
      },
      "type": "core/log"
    },
    "start":
    {
      "outputs":
      {
        "output":
        {
          "connections":
          [
            {
              "element": "log",
              "input": "trigger"
            }
          ]
        }
      },
      "type": "trigger/start"
    }
  }
}
  )"};
  ObTools::JSON::Parser parser(iss);
  ObTools::JSON::Value value;
  try
  {
    value = parser.read_value();
  }
  catch (ObTools::JSON::Exception& e)
  {
    log.error << "JSON parsing failed: " << e.error << endl;
  }

  try
  {
    ViGraph::JSON::set(engine, value, string{});
  }
  catch (runtime_error& e)
  {
    log.error << "Graph load failed: " << e.what() << endl;
  }

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
