//==========================================================================
// Main file for ViGraph engine desktop application
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "desktop.h"
#include "vg-service.h"
#include <QApplication>
#include <QStandardPaths>
#include <QWebView>
#include "ot-daemon.h"

using namespace ViGraph::Engine;

namespace {
const auto server_name    = "ViGraph dataflow desktop application";
const auto server_version = VERSION;

#if defined(PLATFORM_WINDOWS)
const auto application_name = "ViGraph";
#else
const auto application_name = "vigraph";
#endif
const auto application_url = "http://localhost:33380/";

//const auto application_dir = "vigraph";
const auto default_licence = "licence.xml";
const auto default_config = "desktop.cfg.xml";
const auto config_file_root = "desktop";
const auto default_log = "desktop.log";
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
  QApplication::setApplicationName(application_name);
  QApplication app{argc, argv};
  auto config_dir = File::Directory{QStandardPaths::writableLocation(
                      QStandardPaths::AppConfigLocation).toUtf8().constData()};
  auto data_dir = File::Directory{QStandardPaths::writableLocation(
                      QStandardPaths::AppDataLocation).toUtf8().constData()};
  auto licence = config_dir.resolve(File::Path{default_licence}).str();
  auto config = config_dir.resolve(File::Path{default_config}).str();
  auto log = data_dir.resolve(File::Path{default_log}).str();
  MT::Semaphore started;
  Server server(licence, app, started);
  Daemon::Shell shell(server, server_name, server_version,
                      config, config_file_root,
                      log, {});
  auto t = thread{[&]()
  {
    shell.start(argc, argv);
  }};
  started.wait();
  WebPage webpage;
  QWebView webview;
  webview.setPage(&webpage);
  webview.setUrl(QUrl{application_url});
  webview.show();
  const int result = app.exec();
  shell.shutdown();
  t.join();
  return result;
}
