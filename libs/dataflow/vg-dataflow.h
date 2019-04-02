//==========================================================================
// ViGraph dataflow machines: vg-dataflow.h
//
// Generic dataflow graph structures
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VG_DATAFLOW_H
#define __VG_DATAFLOW_H

#include <map>
#include <set>
#include <string>
#include <functional>
#include <cmath>
#include "ot-xml.h"
#include "ot-mt.h"
#include "ot-init.h"
#include "ot-text.h"
#include "ot-time.h"
#include "ot-file.h"
#include "ot-json.h"
#include "ot-log.h"

namespace ViGraph { namespace Dataflow {

// Make our lives easier without polluting anyone else
using namespace std;
using namespace ObTools;
using namespace ViGraph;

// Typedefs
typedef double timestamp_t; // Relative timestamp

//==========================================================================
// Tick data - data that is passed for each tick
struct TickData
{
  timestamp_t t = 0.0;        // Relative timestamp
  uint64_t n = 0;             // Relative tick number
  Time::Duration interval;    // Interval of the tick
  timestamp_t global_t = 0.0; // Absolute timestamp
  uint64_t global_n = 0;      // Absolute tick number

  // Constructors
  TickData() {}
  TickData(timestamp_t _t, uint64_t _n, const Time::Duration& _interval):
    t{_t}, n{_n}, interval{_interval}, global_t{_t}, global_n{_n}
  {}
  TickData(timestamp_t _t, uint64_t _n, const Time::Duration& _interval,
           timestamp_t _global_t, uint64_t _global_n):
    t{_t}, n{_n}, interval{_interval}, global_t{_global_t}, global_n{_global_n}
  {}

  //------------------------------------------------------------------------
  // Get number of samples required for this tick
  unsigned samples(double sample_rate) const
  {
    const auto last_tick_total = static_cast<uint64_t>(
      floor(interval.seconds() * (global_n) * sample_rate));
    const auto tick_total = static_cast<uint64_t>(
      floor(interval.seconds() * (global_n + 1) * sample_rate));
    return tick_total - last_tick_total;
  }

  //------------------------------------------------------------------------
  // Get the current sample position (local)
  uint64_t sample_pos(double sample_rate) const
  {
    const auto last_tick_total = static_cast<uint64_t>(
      floor(interval.seconds() * (global_n) * sample_rate));
    const auto first_local_tick_total = static_cast<uint64_t>(
      floor(interval.seconds() * (global_n - n) * sample_rate));
    return last_tick_total - first_local_tick_total;
  }

  //------------------------------------------------------------------------
  // Get the current sample position based on a start-of-time TickData
  uint64_t sample_pos(double sample_rate, const TickData &start_td) const
  {
    const auto last_tick_total = static_cast<uint64_t>(
      floor(interval.seconds() * (global_n) * sample_rate));
    const auto first_local_tick_total = static_cast<uint64_t>(
      floor(interval.seconds() * start_td.global_n * sample_rate));
    return last_tick_total - first_local_tick_total;
  }
};

//==========================================================================
// Graph data block - carrier for arbitrary data
// Will be specialised to actually hold data in user's subclasses
struct Data
{
  // Minimum to get a vtable
  virtual ~Data() {}
};

class DataPtr: public shared_ptr<Data>
{
 public:
  using shared_ptr<Data>::shared_ptr;

  // Template to check for a given subtype
  // Throws a runtime_error if wrong
  template <class T> shared_ptr<T> check()
  {
    auto t = dynamic_pointer_cast<T>(*this);
    if (!t) throw runtime_error("Bad data type");
    return t;
  }
};

//==========================================================================
// Graph control value
struct Value
{
  enum class Type
  {
    invalid,      // Unset

    // Types for dynamic properties
    trigger,      // No data, just a trigger pulse
    number,       // Real number (also used for integer and boolean >= 0.5)
    text,         // Text string

    // Types only used for static configuration properties
    boolean,       // True / False
    choice,        // Fixed choice set
    file,          // Filename
    other,         // Special type defined separately
    any,           // Set at run time
  };

  // If only enum classes could have members ;-)
  // Get a string representation of a value type
  static string type_str(Value::Type t);

  Type type{Type::invalid};

  // One of the following - can't be in a union because of string
  double d{0.0};
  string s;

  // Constructors
  Value(): type(Type::trigger) {}
  Value(Type _type): type(_type) {}
  Value(double _d): type(Type::number), d(_d) {}
  Value(const string& _s): type(Type::text), s(_s) {}

  // Comparison operators
  bool operator<(const Value& b) const
  {
    return type < b.type || d < b.d || s < b.s;
  }
  bool operator==(const Value& b) const
  {
    return type == b.type && d == b.d && s == b.s;
  }
};

class Graph;  // forward
class Engine;
class Element;

//==========================================================================
// Module metadata
struct Module
{
  string id;                   // Short ID (for config)
  string name;                 // Human name
  string description;          // Long description
  string section;              // Section name

  // Properties of this element
  struct PropertyDescription
  {
    string description;
    string default_value;
    PropertyDescription(const string& _desc, const string& _def=""):
      description(_desc), default_value(_def) {}
    PropertyDescription(const char *_desc):  // Allow direct set from string
      description(_desc) {}
  };

  struct Property
  {
    PropertyDescription desc;
    Value::Type type;
    string other_type;  // If type == other

    // Member accessor data
    struct Member
    {
      typedef double Element::*MemberDouble;
      typedef double (Element::*MemberGetDouble)() const ;
      typedef void (Element::*MemberSetDouble)(double);
      typedef string Element::*MemberString;
      typedef string (Element::*MemberGetString)() const ;
      typedef void (Element::*MemberSetString)(const string&);
      typedef bool Element::*MemberBool;
      typedef bool (Element::*MemberGetBool)() const ;
      typedef void (Element::*MemberSetBool)(bool);
      typedef int Element::*MemberInt;
      typedef int (Element::*MemberGetInt)() const ;
      typedef void (Element::*MemberSetInt)(int);
      typedef JSON::Value (Element::*MemberGetJSON)() const ;
      typedef void (Element::*MemberSetJSON)(const JSON::Value&);
      typedef void (Element::*MemberTrigger)();

      // Simple member pointers
      MemberDouble d_ptr{nullptr};
      MemberString s_ptr{nullptr};
      MemberBool b_ptr{nullptr};
      MemberInt i_ptr{nullptr};

      // Getter/setter functions
      MemberGetDouble get_d{nullptr};
      MemberGetString get_s{nullptr};
      MemberGetBool get_b{nullptr};
      MemberGetInt get_i{nullptr};
      MemberGetJSON get_json{nullptr};

      MemberSetDouble set_d{nullptr};
      MemberSetString set_s{nullptr};
      MemberSetBool set_b{nullptr};
      MemberSetInt set_i{nullptr};
      MemberSetJSON set_json{nullptr};

      // Trigger function
      MemberTrigger trigger{nullptr};

      // Constructors to set each of the above
      Member() {}
      template<typename T>
      Member(double T::*_p): d_ptr(static_cast<MemberDouble>(_p)) {}
      template<typename T>
      Member(string T::*_p): s_ptr(static_cast<MemberString>(_p)) {}
      template<typename T>
      Member(bool T::*_p): b_ptr(static_cast<MemberBool>(_p)) {}
      template<typename T>
      Member(int T::*_p): i_ptr(static_cast<MemberInt>(_p)) {}

      template<typename T>
      Member(double (T::*_get)() const, void (T::*_set)(double)):
        get_d(static_cast<MemberGetDouble>(_get)),
        set_d(static_cast<MemberSetDouble>(_set)) {}
      template<typename T>
      Member(void (T::*_set)(double)):
        set_d(static_cast<MemberSetDouble>(_set)) {}
      template<typename T>
      Member(string (T::*_get)() const, void (T::*_set)(const string&)):
        get_s(static_cast<MemberGetString>(_get)),
        set_s(static_cast<MemberSetString>(_set)) {}
      template<typename T>
      Member(bool (T::*_get)() const, void (T::*_set)(bool)):
        get_b(static_cast<MemberGetBool>(_get)),
        set_b(static_cast<MemberSetBool>(_set)) {}
      template<typename T>
      Member(int (T::*_get)() const, void (T::*_set)(int)):
        get_i(static_cast<MemberGetInt>(_get)),
        set_i(static_cast<MemberSetInt>(_set)) {}
      template<typename T>
      Member(JSON::Value (T::*_get)() const,
             void (T::*_set)(const JSON::Value&)):
        get_json(static_cast<MemberGetJSON>(_get)),
        set_json(static_cast<MemberSetJSON>(_set)) {}

      template<typename T>
      Member(void (T::*_f)()): trigger(static_cast<MemberTrigger>(_f)) {}
    } member;

    set<string> options;       // Options for choice
    bool settable{false};      // Can be connected to and changed dynamically
    bool alias{false};         // Another way of representing an earlier value
    // Constructors
    Property(const PropertyDescription& _desc, Value::Type _type,
             const Member& _member, bool _set=false, bool _alias=false):
      desc(_desc), type(_type), member(_member), settable(_set),
      alias(_alias) {}

    // choice value
    Property(const PropertyDescription& _desc, Value::Type _type,
             const Member& _member,
             const set<string>& _options, bool _set=false):
      desc(_desc), type(_type), member(_member),
        options(_options), settable(_set) {}

    // complex value ('other')
    Property(const PropertyDescription& _desc, const string& _type,
             const Member& _member, bool _set=false):
      desc(_desc), type(Value::Type::other), other_type(_type),
        member(_member), settable(_set) {}
  };

  map<string, Property> properties;

  // Properties controlled on targets
  struct ControlledProperty
  {
    string description;
    string name;        // Default name to connect to
    Value::Type type;
    ControlledProperty(const string& _desc, const string& _name, Value::Type _type):
      description(_desc), name(_name), type(_type) {}
  };

  map<string, ControlledProperty> controlled_properties;  // by our name

  // Data inputs
  struct Input
  {
    string type;
    bool multiple{false};
    Input(const string& _type, bool _m=false): type(_type), multiple(_m) {}
    Input(const char *_type): type(_type) {}
  };

  list<Input> inputs;

  // Data outputs
  struct Output
  {
    string type;
    bool multiple{false};
    Output(const string& _type, bool _m=false): type(_type), multiple(_m) {}
    Output(const char *_type): type(_type) {}
  };

  list<Output> outputs;

  bool is_container{false};

  // Constructors
  // For controls, with controlled properties
  Module(const string& _id, const string& _name, const string& _desc,
         const string& _section,
         const map<string, Property>& _props,
         const map<string, ControlledProperty>& _cprops):
    id(_id), name(_name), description(_desc), section(_section),
    properties(_props), controlled_properties(_cprops) {}

  // For filters etc. with inputs/outputs
  Module(const string& _id, const string& _name, const string& _desc,
         const string& _section,
         const map<string, Property>& _props,
         const list<Input>& _inputs,
         const list<Output>& _outputs,
         bool _is_container = false):
    id(_id), name(_name), description(_desc), section(_section),
    properties(_props),
    inputs(_inputs), outputs(_outputs),
    is_container(_is_container) {}

  // For filter+controls with both
  Module(const string& _id, const string& _name, const string& _desc,
         const string& _section,
         const map<string, Property>& _props,
         const map<string, ControlledProperty>& _cprops,
         const list<Input>& _inputs,
         const list<Output>& _outputs,
         bool _is_container = false):
    id(_id), name(_name), description(_desc), section(_section),
    properties(_props),
    controlled_properties(_cprops),
    inputs(_inputs), outputs(_outputs),
    is_container(_is_container) {}

  // For services, with neither
  Module(const string& _id, const string& _name, const string& _desc,
         const string& _section,
         const map<string, Property>& _props):
    id(_id), name(_name), description(_desc), section(_section),
    properties(_props) {}
};

//==========================================================================
// Graph element - just has an ID and a parent graph
class Element
{
private:
  void configure_from_element(const XML::Element& config,
                              const string& prefix);
  void configure_property(const string& name, const Module::Property& prop,
                          const string& value);
  void set_property(const string& prop_name, const Module::Property& prop,
                    const Value& v);
  JSON::Value get_property_json(const Module::Property& prop) const;

public:
  const Module *module{nullptr};
  string id;
  Graph *graph{nullptr};
  Engine *engine{nullptr};
  Element *next_element{nullptr};
  list<Element *> downstreams; // All data and control connections, for toposort

  // Basic construction with XML - just sets ID
  // Extend this to read basic config which doesn't take much time or
  // require registry - i.e. just reading basic config attributes
  Element(const Module *_module, const XML::Element& config):
    module(_module), id(config["id"]) {}

  // Configure with XML config
  // Throws a runtime_error if configuration fails
  // !!! Should be able to make this final once all automated
  virtual void configure(const File::Directory& base_dir,
                         const XML::Element& config);

  // Setup after automatic configuration
  virtual void setup(const File::Directory& /*base_dir*/) { setup(); }
  virtual void setup() {}

  // Connect to other elements in the graph, for cases where the normal
  // graph connection isn't sufficient.  Called when the graph is already
  // loaded and all elements configured, and after normal connection
  virtual void connect() {}

  // Notify that this element is the control target of another element,
  // with the given property name
  virtual void notify_target_of(const string& /*prop*/) {}

  // Get state as JSON - path is XPath-like path to subelements - ignore
  // in leaf elements (when it should be empty anyway)
  virtual JSON::Value get_json(const string& path="") const;

  // Set a control value
  virtual void set_property(const string& property, const Value&);

  // Update after setting a property
  virtual void update() {}

  // Get type of a control property - uses module by default but overridable
  // for testing
  virtual Value::Type get_property_type(const string& property);

  // Notify of parent graph being enabled - register for keys etc.
  virtual void enable() {}

  // Notify of parent graph being disabled - de-register for keys etc.
  virtual void disable() {}

  // Notify of a new tick about to start
  virtual void pre_tick(const TickData&) {}

  // Tick
  virtual void tick(const TickData&) {}

  // Notify of a tick just ended
  virtual void post_tick(const TickData&) {}

  // Clean shutdown
  virtual void shutdown() {}

  // Virtual destructor
  virtual ~Element() {}
};

class Generator;  // forward

//==========================================================================
// Acceptor interface (mixin)
class Acceptor
{
 public:
  // Accept data
  virtual void accept(DataPtr data) = 0;

  virtual ~Acceptor() {}
};

//==========================================================================
// Generator - something that generates data on a single output
class Generator: public Element
{
 protected:
  string acceptor_id;   // If specified
  Acceptor *acceptor{nullptr};

 public:
  // Constructor - get acceptor_id as well as Element stuff
  Generator(const Module *_module, const XML::Element& config):
    Element(_module, config), acceptor_id(config["acceptor"]) {}

  // Get the acceptor ID
  const string& get_acceptor_id() { return acceptor_id; }

  // Set the acceptor and optionally ID - note either are overridable,
  // second downcalls to first
  virtual void attach(Acceptor *_acceptor)
  { acceptor = _acceptor; }
  virtual void attach(Acceptor *_acceptor, const string& _acceptor_id)
  { attach(_acceptor); acceptor_id = _acceptor_id; }

  // Send data down
  void send(DataPtr data) { if (acceptor) acceptor->accept(data); }
  // Sugar to allow direct send of 'new Data' in sources
  void send(Data *data) { send(DataPtr(data)); }

  // Get state as JSON
  JSON::Value get_json(const string& path="") const override;
};

//==========================================================================
// Filter - takes in data and modifies it, passing on to a single output
class Filter: public Generator, public Acceptor
{
 public:
  using Generator::Generator;
};

//==========================================================================
// Initial source - Generator with a tick()
class Source: public Generator
{
public:
  using Generator::Generator;
};

//==========================================================================
// Final sink - Element with Acceptor
class Sink: public Element, public Acceptor
{
 public:
  using Element::Element;
};

//==========================================================================
// ControlImpl - implementation mixin for Control
class ControlImpl
{
  string control_id;

 public:
  struct Property
  {
    string name;  // Target property name
    Value::Type type{Value::Type::invalid};
    bool is_explicit;

    Property() {}
    Property(const string& _name, Value::Type _type, bool _is_explicit=false):
      name(_name), type(_type), is_explicit(_is_explicit) {}
  };

  struct Target
  {
    map<string, Property> properties;  // Our name -> property name/type
    Element *element{nullptr};
  };

 protected:
  XML::Element config;
  map<string, Target> targets;  // By element ID

 public:
  // Construct with XML
  ControlImpl(const Module *_module, const XML::Element& _config,
              bool targets_are_optional = false);

  // Get targets
  const map<string, Target>& get_targets() { return targets; }

  // Attach to a target element
  void attach_target(const string& prop_id,
                     Element *target_element,
                     Element *source_element);

  // Send a value to the target using only (first) property
  void send(const Value& v);

  // Send a named value to the target
  // name is our name for it
  void send(const string& name, const Value& v);

  // Trigger first target property
  void trigger() { send({}); }

  // Trigger named property
  void trigger(const string& name) { send(name, {}); }

  // Get state as JSON, adding to the given value
  void add_json(JSON::Value& json) const;
};

//==========================================================================
// Control - sets one or more properties on a target Element
class Control: public Element, public ControlImpl
{
 public:
  Control(const Module *_module, const XML::Element& _config,
          bool targets_are_optional = false):
    Element(_module, _config),
    ControlImpl(_module, _config, targets_are_optional)
  {}

 private:
  // Hide these tick calls because Controls should do all their work in the
  // pre-tick phase
  void tick(const TickData&) final {}
  void post_tick(const TickData&) final {}

  // Add control JSON
  JSON::Value get_json(const string &path="") const override
  { JSON::Value json=Element::get_json(path); add_json(json); return json; }
};

//==========================================================================
// Dataflow graph structure
class Graph
{
  Engine& engine;
  mutable MT::RWMutex mutex;
  map<string, shared_ptr<Element> > elements;   // By ID

  // Source
  File::Path source_file;  // empty if inline
  time_t source_file_mtime;
  Time::Duration file_update_check_interval;
  Time::Stamp last_file_update_check;

  // Construction state
  map<string, int> id_serials;  // ID serial number for each type
  list<Element *> disconnected_acceptors;
  list<Generator *> unbound_generators;
  Element *last_element{nullptr};
  Acceptor *external_acceptor{nullptr};

  // Topological ordering - ensure a precursor is ticked before its
  // dependents - either for base data flow or control flow
  list<Element *> topological_order;

  // Temporary state
  bool is_enabled{false};
  map<string, Value> variables;

  // Internals
  void configure_from_source_file();
  void configure_internal(const File::Directory& base_dir,
                          const XML::Element& config);
  void toposort(Element *e, set<Element *>& visited);

 public:
  //------------------------------------------------------------------------
  // Constructor
  Graph(Engine& _engine): engine(_engine) {}

  //------------------------------------------------------------------------
  // Get engine
  Engine& get_engine() const
  { return engine; }

  //------------------------------------------------------------------------
  // Get all elements (for inspection)
  const map<string, shared_ptr<Element> >& get_elements() const
  { return elements; }

  //------------------------------------------------------------------------
  // Configure with XML, with a base directory for files
  // Throws a runtime_error if configuration fails
  void configure(const File::Directory& base_dir, const XML::Element& config);

  //------------------------------------------------------------------------
  // Configure from source file, with given update check interval
  void configure(const File::Path& source_file,
                 const Time::Duration& check_interval,
                 Acceptor *external_acceptor);

  //------------------------------------------------------------------------
  // Set an element property
  // element_path is a path/to/leaf
  // Can throw runtime_error if it fails
  void set_property(const string& element_path, const string& property,
                    const Value& value);

  //------------------------------------------------------------------------
  // Add an element to the graph
  void add(Element *el);

  //------------------------------------------------------------------------
  // Connect an element in the graph
  // Uses internal state to work out how to connect it:
  //   Acceptors are connected to all previous unconnected Generators
  //   Controls are connected to the last non-control Element
  // Throws runtime_error if it can't connect properly
  void connect(Element *el);

  //------------------------------------------------------------------------
  // Attach a pure Acceptor to all unbound generators remaining in the graph
  // Returns whether any were attached
  // Note, doesn't add to graph ordering and remembers this for reload
  bool attach(Acceptor *a);

  //------------------------------------------------------------------------
  // Attach an Acceptor Element to all unbound generators remaining in the graph
  // Returns whether it is an Acceptor and any were attached
  bool attach(Element *el);

  //------------------------------------------------------------------------
  // Generate topological order - ordered list of elements which ensures
  // a precursor (upstream) element is ticked before its dependents
  // (downstreams)
  // (called automatically by configure() - use only for direct testing)
  void generate_topological_order();

  //------------------------------------------------------------------------
  // Set a variable
  void set_variable(const string& var, const Value& value)
  { variables[var] = value; }

  //------------------------------------------------------------------------
  // Get a variable
  Value get_variable(const string& var)
  { const auto it = variables.find(var);
    return (it == variables.end())?Value(Value::Type::invalid):it->second; }

  //------------------------------------------------------------------------
  // Enable all elements
  void enable();

  //------------------------------------------------------------------------
  // Disable all elements
  void disable();

  //------------------------------------------------------------------------
  // Pre-tick all elements
  void pre_tick(const TickData& td);

  //------------------------------------------------------------------------
  // Tick all elements
  void tick(const TickData& td);

  //------------------------------------------------------------------------
  // Post-tick all elements
  void post_tick(const TickData& td);

  //------------------------------------------------------------------------
  // Get a particular element by ID
  Element *get_element(const string& id);

  //------------------------------------------------------------------------
  // Get state as a JSON value - array for top-level graph, single
  // value for sub-element property
  // Path is an XPath-like list of subgraph IDs and leaf element, or empty
  // for entire graph
  JSON::Value get_json(const string& path="") const;

  //------------------------------------------------------------------------
  // Does this require an update? (i.e. there is a new config)
  bool requires_update(File::Path &file, Time::Duration& check_interval,
                       Acceptor *& external_acceptor);

  //------------------------------------------------------------------------
  // Shutdown all elements
  void shutdown();
};

//==========================================================================
// Thread Pool interface
class ThreadPool
{
public:
  // Run a function on the first available thread
  virtual bool run(function<void()> f) = 0;

  // Run a set of functions in parallel and wait for them all to complete
  virtual void run_and_wait(vector<function<void()>>& vf) = 0;

  // Virtual destructor
  ~ThreadPool() {}
};

//==========================================================================
// MultiGraph - generic container for multiple sub-graphs
// Used for (e.g.) sub-graph selectors
class MultiGraph
{
  Engine& engine;
  MT::RWMutex mutex;
  vector<shared_ptr<Graph> > subgraphs;          // Owning
  map<string, Graph *> subgraphs_by_id;          // Not owning
  int id_serial{0};

  // Thread
  class Thread
  {
  private:
    shared_ptr<Graph> graph;
    TickData td;
    enum class Task
    {
      pre_tick,
      tick,
      post_tick,
      exit,
    } task = Task::exit;
    MT::Condition start_c;
    MT::Condition done_c;
    thread t; // This must be after the conditions

    //----------------------------------------------------------------------
    // Run a task
    void run(const TickData& _td, Task _task);

    //----------------------------------------------------------------------
    // Main thread loop
    void loop();

  public:
    //----------------------------------------------------------------------
    // Constructor
    Thread(shared_ptr<Graph> _graph):
      graph{_graph}, t{&Thread::loop, this}
    {}

    //----------------------------------------------------------------------
    // Run pre-tick
    void pre_tick(const TickData& _td)
    {
      run(_td, Task::pre_tick);
    }

    //----------------------------------------------------------------------
    // Run tick
    void tick(const TickData& _td)
    {
      run(_td, Task::tick);
    }

    //----------------------------------------------------------------------
    // Run post-tick
    void post_tick(const TickData& _td)
    {
      run(_td, Task::post_tick);
    }

    //----------------------------------------------------------------------
    // Wait for run to finish
    void wait();

    //----------------------------------------------------------------------
    // Destructor
    ~Thread();
  };

  // Serialiser for accepts when using thread pool
  class ThreadSerialiser: public Acceptor
  {
  private:
    MT::Mutex mutex;
    Acceptor *external_acceptor = nullptr;

  public:
    //----------------------------------------------------------------------
    // Accept data
    void accept(DataPtr data)
    {
      MT::Lock lock{mutex};
      if (external_acceptor)
        external_acceptor->accept(data);
    }

    //----------------------------------------------------------------------
    // Attach a pure Acceptor (for testing only)
    void attach(Acceptor *a)
    {
      external_acceptor = a;
    }

    //----------------------------------------------------------------------
    // Attach an Acceptor Element
    void attach(Element *el)
    {
      external_acceptor = dynamic_cast<Acceptor *>(el);
    }
  };

  bool threaded = false;

  unique_ptr<ThreadSerialiser> thread_serialiser;
  map<Graph *, Thread> threads;

 public:
  //------------------------------------------------------------------------
  // Constructor
  MultiGraph(Engine& _engine): engine(_engine) {}

  //------------------------------------------------------------------------
  // Configure with XML
  // Reads <graph> child elements of config
  // Throws a runtime_error if configuration fails
  void configure(const File::Directory& base_dir,
                 const XML::Element& config);

  //------------------------------------------------------------------------
  // Add a graph from the given XML
  // Throws a runtime_error if configuration fails
  // Returns sub-Graph (owned by us)
  Graph *add_subgraph(const File::Directory& base_dir,
                      const XML::Element& graph_config);

  //------------------------------------------------------------------------
  // Attach a pure Acceptor to the end of all subgraphs (for testing only)
  void attach_to_all(Acceptor *a);

  //------------------------------------------------------------------------
  // Attach an Acceptor Element to the end of all subgraphs
  void attach_to_all(Element *el);

  //------------------------------------------------------------------------
  // Enable all subgraphs
  void enable_all();

  //------------------------------------------------------------------------
  // Disable all subgraphs
  void disable_all();

  //------------------------------------------------------------------------
  // Pre-tick all subgraphs
  void pre_tick_all(const TickData& td);

  //------------------------------------------------------------------------
  // Tick all subgraphs
  void tick_all(const TickData& td);

  //------------------------------------------------------------------------
  // Post-tick all subgraphs
  void post_tick_all(const TickData& td);

  //------------------------------------------------------------------------
  // Get a particular graph by ID
  Graph *get_subgraph(const string& id);

  //------------------------------------------------------------------------
  // Get a particular graph by index
  Graph *get_subgraph(size_t index);

  //------------------------------------------------------------------------
  // Get all subgraphs
  const map<string, Graph *>& get_subgraphs() { return subgraphs_by_id; }

  //------------------------------------------------------------------------
  // Shutdown all subgraphs
  void shutdown();
};

//==========================================================================
// Generic singleton service - used to provide global services, looked up
// from an Engine by ID and then dynamic_cast to whatever is required
class Service: public Element
{
 public:
  using Element::Element;
};

//==========================================================================
// Registry of Element modules
class Registry
{
public:
  // Abstract interface for Element-creating factories
  struct Factory
  {
    virtual Element *create(const Module *module,
                            const XML::Element& config) const = 0;
    virtual ~Factory() {}
  };

  // Template for factories creating with { new Type }
  template<class E> struct NewFactory: public Factory
  {
  public:
    Element *create(const Module *module, const XML::Element& config) const
    { return new E(module, config); }
  };

  struct ModuleInfo
  {
    const Module *module;
    const Factory *factory;
    ModuleInfo() {}
    ModuleInfo(const Module *_module, const Factory *_factory):
      module(_module), factory(_factory) {}
  };
  map<string, ModuleInfo> modules;

  //------------------------------------------------------------------------
  // Constructor
  Registry() {}

  //------------------------------------------------------------------------
  // Register a module with its factory
  void add(const Module& m, const Factory& f)
  { modules[m.id] = ModuleInfo(&m, &f); }

  //------------------------------------------------------------------------
  // Create an object by module and config
  // Returns the object, or 0 if no factories available or create fails
  Element *create(const string& name, const XML::Element& config)
  {
    const auto p = modules.find(name);
    if (p!=modules.end())
    {
      const auto& mi = p->second;
      return mi.factory->create(mi.module, config);
    }
    else
      return 0;
  }
};

//==========================================================================
// Engine class - wrapper containing Graph tree and Element/Service registries
class Engine
{
  // Services
  map<string, shared_ptr<Element>> services;

  // Graph structure
  mutable MT::RWMutex graph_mutex;
  unique_ptr<Dataflow::Graph> graph;
  Time::Duration tick_interval{0.04};  // 25Hz default
  Time::Stamp start_time;
  uint64_t tick_number{0};

 public:
  Registry element_registry;
  Registry service_registry;

  //------------------------------------------------------------------------
  // Constructor
  Engine(): graph(new Graph(*this)) {}

  //------------------------------------------------------------------------
  // Set/get the tick interval
  void set_tick_interval(const Time::Duration& d) { tick_interval = d; }
  Time::Duration get_tick_interval() const { return tick_interval; }

  //------------------------------------------------------------------------
  // Get the graph (for testing only)
  Dataflow::Graph& get_graph() { return *graph; }

  //------------------------------------------------------------------------
  // Get a type-checked service
  template <class T> shared_ptr<T> get_service(const string& id)
  {
    const auto it = services.find(id);
    if (it == services.end())
      throw runtime_error("No such service "+id);

    auto t = dynamic_pointer_cast<T>(it->second);
    if (!t) throw runtime_error("Service "+id+" is the wrong type");
    return t;
  }

  //------------------------------------------------------------------------
  // Configure with <graph> and <services> XML
  // Throws a runtime_error if configuration fails
  void configure(const File::Directory& base_dir,
                 const XML::Element& graph_config,
                 const XML::Element& services_config);

  //------------------------------------------------------------------------
  // Set an element property
  // element_path is a path/to/leaf
  void set_property(const string& element_path, const string& property,
                    const Value& value);

  //------------------------------------------------------------------------
  // Get state as a JSON value (see Graph::get_json())
  JSON::Value get_json(const string& path) const;

  //------------------------------------------------------------------------
  // Tick the graph
  void tick(Time::Stamp t);

  //------------------------------------------------------------------------
  // Shut down the graph
  void shutdown();
};


//==========================================================================
}} //namespaces
#endif // !__VG_DATAFLOW_H
