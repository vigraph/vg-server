//==========================================================================
// Main file for ViGraph engine desktop application
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "desktop.h"
#include "vg-service.h"
#include <QApplication>
#include <QWebView>
#include "ot-daemon.h"

using namespace ViGraph::Engine;

namespace {
const auto server_name    = "ViGraph dataflow engine daemon";
const auto server_version = VERSION;

const auto application_url = "http://localhost:33380/";

#if defined(PLATFORM_WINDOWS)
const auto default_licence = "licence.xml";
const auto default_config_file = "engine.cfg.xml";
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

class WebPage: public QWebPage
{
public:
  using QWebPage::QWebPage;

  void javaScriptConsoleMessage(const QString& message, int line_number,
                                const QString& source_id)
  {
    Log::Detail detail;
    detail << "JS (" << source_id.toUtf8().constData()
           << ":" << line_number
           << "): " << message.toUtf8().constData() << endl;
  }
};

int main(int argc, char **argv)
{
  QApplication app{argc, argv};
  auto t = thread{[&]()
  {
#if defined(PLATFORM_WINDOWS)
    winsock_initialise();
    wchar_t p[MAX_PATH];
    GetModuleFileNameW(nullptr, p, MAX_PATH);
    const auto path = File::Path(Text::UTF8::encode(&p[0]));
    const auto licence_file = path.dirname() + "\\" + default_licence;
    const auto config_file = path.dirname() + "\\" + default_config_file;
    Server server(licence_file, app);
    Daemon::Shell shell(server, server_name, server_version,
                        config_file, config_file_root,
                        default_log_file, pid_file);
#else
    Server server(default_licence, app);
    Daemon::Shell shell(server, server_name, server_version,
                        default_config_file, config_file_root,
                        default_log_file, pid_file);
#endif
    shell.start(argc, argv);
  }};
  WebPage webpage;
  QWebView webview;
  webview.setPage(&webpage);
  webview.setUrl(QUrl{application_url});
  webview.show();
  const int result = app.exec();
  t.join();
  return result;
}
