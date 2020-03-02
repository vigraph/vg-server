//==========================================================================
// Internal definitions for dataflow service
//
// Copyright (c) 2017-2020 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VG_SERVICE_H
#define __VG_SERVICE_H

#include "ot-soap.h"
#include "ot-lib.h"
#include "vg-dataflow.h"
#include "vg-geometry.h"

namespace ViGraph { namespace Service {

// Make our lives easier without polluting anyone else
using namespace std;
using namespace ObTools;
using namespace ViGraph;

// Module vg_init function interface
typedef bool vg_init_fn_t(Log::Channel&, Dataflow::Engine&);

//==========================================================================
// Layout Holder
class Layout
{
private:
  mutable MT::Mutex mutex;
  string layout;

public:
  string get_layout() const
  {
    MT::Lock lock{mutex};
    return layout;
  }

  void set_layout(const string& new_layout)
  {
    MT::Lock lock{mutex};
    layout = new_layout;
  }
};

//==========================================================================
// Abstract main thread runner interface
class MainThreadRunner
{
public:
  //------------------------------------------------------------------------
  // Run a function on the main thread
  virtual future<bool> run_function(function<void()> func) = 0;

  //------------------------------------------------------------------------
  // Destructor
  virtual ~MainThreadRunner() {}
};

//==========================================================================
// REST Interface
class RESTInterface
{
private:
  unique_ptr<Web::SimpleHTTPServer> http_server;
  unique_ptr<Net::TCPServerThread> http_server_thread;

  Layout layout;

public:
  RESTInterface(const XML::Element& config, Dataflow::Engine& _engine,
                MainThreadRunner& runner, const File::Directory& base_dir);
  ~RESTInterface();
};

//==========================================================================
// File Server
class FileServer
{
  unique_ptr<Web::SimpleHTTPServer> http_server;
  unique_ptr<Net::TCPServerThread> http_server_thread;

public:
  FileServer(const XML::Element& config, const File::Directory& base_dir);
  ~FileServer();
};

//==========================================================================
// Global state
// Singleton instance of server-wide state
class Server: public MainThreadRunner
{
  XML::Element config_xml;
  File::Path config_file;
  string licence_file;
  File::Path graph_file;
  time_t graph_file_time = 0;

  // Loaded modules
  struct Module
  {
    unique_ptr<Lib::Library> lib;
    Time::Stamp mtime;
    Module() {}
    Module(Lib::Library *_lib, const Time::Stamp& _t): lib(_lib), mtime(_t) {}
  };
  map<string, Module> modules;   // By pathname

  // Dataflow engine
  Dataflow::Engine engine;

  // Management interface
  unique_ptr<RESTInterface> rest;

  // Basic file server
  unique_ptr<FileServer> file_server;

  // Function queue
  MT::Mutex functions_mutex;
  struct FutureFunction
  {
    function<void()> func;
    promise<bool> promise;
    FutureFunction(function<void()> _func): func{_func} {}
    auto get_future() { return promise.get_future(); }
  };
  queue<FutureFunction> functions;

  // Internal
  bool load_module(const File::Path& path);

  // Load a graph
  bool load_graph(const File::Path& path);

public:
  //------------------------------------------------------------------------
  // Constructor
  Server(const string& _licence_file);

  //------------------------------------------------------------------------
  // Get the engine
  Dataflow::Engine& get_engine() { cout << "Get engine " << &engine << endl;
    return engine; }

  //------------------------------------------------------------------------
  // Read settings from configuration
  void read_config(const XML::Configuration& config,
                   const string& config_filename);

  //------------------------------------------------------------------------
  // Prerun function for child process
  int run_priv();

  //------------------------------------------------------------------------
  // Pre main loop function
  int pre_run();

  //------------------------------------------------------------------------
  // Main loop iteration function
  int tick();

  //------------------------------------------------------------------------
  // Run a function on the main thread
  future<bool> run_function(function<void()> func) override;

  //------------------------------------------------------------------------
  // Global configuration - called only at startup
  // Returns whether successful
  bool configure();

  //------------------------------------------------------------------------
  // Global pre-configuration - called at startup
  int preconfigure();

  //------------------------------------------------------------------------
  // Global re-configuration - called at SIGHUP
  void reconfigure();

  //------------------------------------------------------------------------
  // Clean up
  void cleanup();

  //------------------------------------------------------------------------
  // Virtual Destructor
  ~Server();
};

//==========================================================================
}} //namespaces
#endif // !__VG_SERVICE_H
