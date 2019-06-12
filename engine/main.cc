//==========================================================================
// ViGraph engine server: main.cc
//
// Main file for ViGraph engine server
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#if defined(PLATFORM_WINDOWS)
#define WEBVIEW_WINAPI
#define WEBVIEW_IMPLEMENTATION
#include <webview.h>
#endif

#include "engine.h"

namespace {
const auto server_name    = "ViGraph dataflow engine daemon";
const auto server_version = VERSION;

#if defined(PLATFORM_WINDOWS)
const auto application_name = "ViGraph Create Pro";
const auto application_url = "http://localhost:33380/";
const auto default_licence_file_name = "licence.xml";
const auto default_config_file_name = "engine.cfg.xml";
#else
const auto default_licence = "/etc/vigraph/licence.xml";
#ifdef DEBUG
const auto default_config_file = "engine.cfg.xml";
#else
const auto default_config_file = "/etc/vigraph/engine.cfg.xml";
#endif
#endif

const auto config_file_root = "engine";
const auto default_log_file = "/var/log/vigraph/engine.log";
const auto pid_file         = "/var/run/vg-engine.pid";
}

using namespace std;
using namespace ObTools;
using namespace ViGraph::Engine;

//--------------------------------------------------------------------------
// Main
#if defined(PLATFORM_WINDOWS)
void webview_callback(struct webview *wv, const char *message)
{
  try
  {
    auto iss = istringstream{message};
    auto parser = JSON::Parser(iss);
    auto value = parser.read_value();
    if (value["fullscreen"].is_true())
      webview_set_fullscreen(wv, true);
    else
      webview_set_fullscreen(wv, false);
  }
  catch (const runtime_error& e)
  {
    Log::Error log;
    log << "Callback from webview failed: " << e.what() << endl;
  }
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
  winsock_initialise();
  wchar_t p[MAX_PATH];
  GetModuleFileNameW(nullptr, p, MAX_PATH);
  const auto path = File::Path(Text::UTF8::encode(&p[0]));
  const auto licence_file = path.dirname() + "\\" + default_licence_file_name;
  const auto config_file = path.dirname() + "\\" + default_config_file_name;
  Server server(licence_file);
  auto on_run = [instance]()
    {
      cout << "onrun" << endl;
      struct webview wv{};
      wv.title = application_name;
      wv.url = application_url;
      wv.width = 800;
      wv.height = 600;
      wv.resizable = true;
      wv.external_invoke_cb = webview_callback;
      webview_init(&wv);
      auto icon = LoadIcon(instance, MAKEINTRESOURCE(1));
      if (icon)
      {
        cout << "Got an icon" << icon << endl;
        SendMessage(wv.priv.hwnd, WM_SETICON, ICON_BIG,
                    reinterpret_cast<LPARAM>(icon));
        SendMessage(wv.priv.hwnd, WM_SETICON, ICON_SMALL,
                    reinterpret_cast<LPARAM>(icon));
        DestroyIcon(icon);
      }
      webview_set_color(&wv, 0, 0, 0, 0);
      webview_set_fullscreen(&wv, true);
      while (!webview_loop(&wv, true))
      {}
    };
  Daemon::WindowsShell shell(server, server_name, server_version, on_run,
                             config_file, config_file_root,
                             default_log_file, pid_file);
  return shell.start(__argc, __argv);
}
#else
int main(int argc, char **argv)
{
  Server server(default_licence);
  Daemon::Shell shell(server, server_name, server_version,
                      default_config_file, config_file_root,
                      default_log_file, pid_file);
  return shell.start(argc, argv);
}
#endif
