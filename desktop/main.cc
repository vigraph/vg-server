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
#include <QDesktopServices>
#include <QMessageBox>
#include "ot-daemon.h"

using namespace ViGraph::Engine;

namespace {
const auto server_name    = "ViGraph dataflow desktop application";
const auto server_version = VERSION;

const auto full_app_name = "ViGraph Create Pro";
#if defined(PLATFORM_WINDOWS)
const auto short_app_name = "ViGraph";
#else
const auto short_app_name = "vigraph";
#endif
const auto website_url = "http://vigraph.com/";
const auto copyright_url = "http://localhost:33380/copyright.html";
const auto application_url = "http://localhost:33380/";

const auto default_licence = "licence.xml";
const auto default_config = "desktop.cfg.xml";
const auto config_file_root = "desktop";
const auto default_log = "desktop.log";

const auto about_text = string{}
  + "<style>"
  + "a:link { color: rgb(0, 208, 224); }"
  + "td { vertical-align: middle; }"
  + "</style>"
  + "<body>"
  + "<table><td><img src=\":/vigraph.ico\"/></td>"
  + "<td>"
  + "<p><strong>" + full_app_name + "</strong></p>"
  + "<p>Version: " + VERSION + "</p>"
  + "<p>Website: <a href=\"" + website_url + "\">" + website_url + "</a></p>"
  + "<p><a href=\"" + copyright_url + "\">Copyright and Licences</a></p>"
  + "</td></table></body>";
const auto about_style = string{}
  + "background-color: black;"
  + "color: white;";
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
  webview.setWindowTitle(full_app_name);
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
  app.setQuitOnLastWindowClosed(false); // otherwise closing about quits app
  QMenu menu{};

  // Edit diagram
  QAction edit_action{"Edit Diagram"};
  QObject::connect(&edit_action, &QAction::triggered, []
  {
    QDesktopServices::openUrl(QUrl{application_url});
  });

  // About
  QAction about_action{"About"};
  QObject::connect(&about_action, &QAction::triggered, [&menu, &icon]
  {
    QMessageBox about{&menu};
    about.setWindowTitle((string{"About "} + full_app_name).c_str());
    about.setWindowIcon(icon);
    about.setTextFormat(Qt::RichText);
    about.setStyleSheet(about_style.c_str());
    about.setText(about_text.c_str());
    about.exec();
  });

  // Quit
  QAction quit_action{"Quit"};
  QObject::connect(&quit_action, &QAction::triggered, [&app]
  {
    app.quit();
  });

  menu.addAction(&edit_action);
  menu.addAction(&about_action);
  menu.addAction(&quit_action);

  QSystemTrayIcon systray{icon};
  systray.setContextMenu(&menu);
  systray.show();

  edit_action.trigger();

  return app.exec();
}

//==========================================================================
// Main
int main(int argc, char **argv)
{
  QApplication::setApplicationName(short_app_name);
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
