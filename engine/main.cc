//==========================================================================
// ViGraph engine server: main.cc
//
// Main file for ViGraph engine server
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#define USE_WEBVIEW 0
#if defined(PLATFORM_WINDOWS)
#if USE_WEBVIEW
#define WEBVIEW_HEADER
#include <webview.h>
#else
#include <windows.h>
#include <shellapi.h>
#endif
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

#if USE_WEBVIEW
void webview_callback(struct webview * /*wv*/, const char *message)
{
  try
  {
    auto iss = istringstream{message};
    auto parser = JSON::Parser(iss);
    auto value = parser.read_value();
    /*
    if (value["fullscreen"].is_true())
      webview_set_fullscreen(wv, true);
    else
      webview_set_fullscreen(wv, false);
      */
  }
  catch (const runtime_error& e)
  {
    Log::Error log;
    log << "Callback from webview failed: " << e.what() << endl;
  }
}

#else

void open_graph_in_browser()
{
  ShellExecute(0, 0, application_url, 0, 0, SW_SHOW);
}

LRESULT CALLBACK handle_window_messages(HWND hwnd, UINT msg,
                                        WPARAM wparam, LPARAM lparam)
{
  enum MenuOption
  {
    menu_edit_graph,
    menu_quit,
  };

  switch (msg)
  {
    case WM_USER + 1:
      switch (LOWORD(lparam))
      {
        case WM_RBUTTONDOWN:
          {
            auto click_point = POINT{};
            GetCursorPos(&click_point);
            auto menu = CreatePopupMenu();
            InsertMenu(menu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING,
                       menu_edit_graph, "Edit Graph");
            InsertMenu(menu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING,
                       menu_quit, "Quit");

            SetForegroundWindow(hwnd);
            TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_LEFTBUTTON |
                                 TPM_BOTTOMALIGN,
                           click_point.x, click_point.y, 0, hwnd, nullptr);
          }
          return TRUE;

        default:
          return DefWindowProc(hwnd, msg, wparam, lparam);
      }
      break;

    case WM_COMMAND:
      {
        switch (LOWORD(wparam))
        {
          case menu_edit_graph:
            open_graph_in_browser();
            break;

          case menu_quit:
            DestroyWindow(hwnd);
            break;

          default:
            return DefWindowProc(hwnd, msg, wparam, lparam);
        }
      }
      break;

    case WM_DESTROY:
      PostQuitMessage(0);
      break;

    default:
      return DefWindowProc(hwnd, msg, wparam, lparam);
  }
  return 0;
}
#endif

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
  auto mh = CreateMutex(nullptr, true, server_name);
  if (GetLastError() == ERROR_ALREADY_EXISTS)
  {
    cerr << "An instance is already running" << endl;
    return 1;
  }
  winsock_initialise();
  wchar_t p[MAX_PATH];
  GetModuleFileNameW(nullptr, p, MAX_PATH);
  const auto path = File::Path(Text::UTF8::encode(&p[0]));
  const auto licence_file = path.dirname() + "\\" + default_licence_file_name;
  const auto config_file = path.dirname() + "\\" + default_config_file_name;
  Server server(licence_file);
  auto on_run = [instance]()
    {
      auto icon = LoadIcon(instance, MAKEINTRESOURCE(1));
#if USE_WEBVIEW
      auto wv = webview_create(true, nullptr);
      webview_set_title(wv, application_name);
      // Setting the size often goes bananas
      //webview_set_size(wv, 800, 600, WEBVIEW_HINT_NONE);
      if (icon)
      {
        auto hwnd = reinterpret_cast<HWND>(webview_get_window(wv));
        SendMessage(hwnd, WM_SETICON, ICON_BIG,
                    reinterpret_cast<LPARAM>(icon));
        SendMessage(hwnd, WM_SETICON, ICON_SMALL,
                    reinterpret_cast<LPARAM>(icon));
      }
      webview_navigate(wv, application_url);
      webview_run(wv);
      webview_destroy(wv);
#else
      auto wc = WNDCLASS{};
      wc.cbClsExtra = 0;
      wc.cbWndExtra = 0;
      wc.hbrBackground = nullptr;
      wc.hCursor = nullptr;
      wc.hIcon = LoadIcon(instance, MAKEINTRESOURCE(1));
      wc.hInstance = instance;
      wc.lpfnWndProc = handle_window_messages;
      wc.lpszClassName = application_name;
      wc.lpszMenuName = nullptr;
      wc.style = 0;
      RegisterClass(&wc);

      auto hwnd = CreateWindow(application_name, application_name,
                               WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0,
                               CW_USEDEFAULT, 0, nullptr, nullptr, instance,
                               nullptr);
      if (!hwnd)
        return;

      auto nid = NOTIFYICONDATA{};
      nid.cbSize = sizeof(nid);
      nid.hWnd = hwnd;
      nid.uID = 1;
      nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
      nid.hIcon = LoadIcon(instance, MAKEINTRESOURCE(1));
      nid.uCallbackMessage = WM_USER + 1;
      lstrcpy(nid.szTip, application_name);
      Shell_NotifyIcon(NIM_ADD, &nid);

      open_graph_in_browser();

      auto msg = MSG{};
      while (GetMessage(&msg, nullptr, 0, 0))
      {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
      }

      Shell_NotifyIcon(NIM_DELETE, &nid);
#endif
      if (icon)
        DestroyIcon(icon);
    };
  Daemon::WindowsShell shell(server, server_name, server_version, on_run,
                             config_file, config_file_root,
                             default_log_file, pid_file);
  auto result = shell.start(__argc, __argv);
  ReleaseMutex(mh);
  CloseHandle(mh);
  return result;
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
