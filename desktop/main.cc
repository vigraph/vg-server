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
#include <QSystemTrayIcon>
#include <QMenu>
#include "ot-daemon.h"

using namespace ViGraph::Engine;

namespace {
const auto server_name    = "ViGraph dataflow desktop application";
const auto server_version = VERSION;

const auto window_name = "ViGraph Create Pro";
#if defined(PLATFORM_WINDOWS)
const auto application_name = "ViGraph";
#else
const auto application_name = "vigraph";
#endif
const auto application_url = "http://localhost:33380/";

const auto default_licence = "licence.xml";
const auto default_config = "desktop.cfg.xml";
const auto config_file_root = "desktop";
const auto default_log = "desktop.log";
}

//==========================================================================
// Full mode

//--------------------------------------------------------------------------
// Custom webpage class for logging javascript console
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

//--------------------------------------------------------------------------
// Full application mode
int run_full(QApplication& app, const QIcon& icon)
{
  WebPage webpage;
  QWebView webview;
  webview.setWindowTitle(window_name);
  webview.setWindowIcon(icon);
  webview.setPage(&webpage);
  webview.setUrl(QUrl{application_url});
  webview.show();
  return app.exec();
}

//==========================================================================
// Systray mode

//--------------------------------------------------------------------------
// System tray application mode
int run_systray(QApplication& app, const QIcon& icon)
{
  QSystemTrayIcon systray{icon};
  QMenu menu{};
  QAction quit_action{"Quit", nullptr};
  QObject::connect(&quit_action, &QAction::triggered, &app, [&app]
  {
    app.quit();
  });
  menu.addAction(&quit_action);
  systray.setContextMenu(&menu);
  systray.show();
  return app.exec();
}

//==========================================================================
// Main
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
    started.signal(); // In case of exit before start up achieved
  }};
  started.wait();
  auto icon = QIcon{":/vigraph.ico"};
  const auto mode = server.get_application_mode();
  int result = 1000;
  switch (mode)
  {
    case Mode::systray:
      result = run_systray(app, icon);
      break;
    case Mode::full:
      result = run_full(app, icon);
      break;
  }
  shell.shutdown();
  t.join();
  return result;
}
