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
#include <cmath>
#include "ot-xml.h"
#include "ot-mt.h"
#include "ot-init.h"
#include "ot-text.h"
#include "ot-time.h"
#include "ot-file.h"

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
    any            // Set at run time
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
};

class Graph;  // forward
class Engine;

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
    string xpath;              // Relative xpath for config
    set<string> options;       // Options for choice
    bool settable{false};
    Property(const PropertyDescription& _desc, Value::Type _type, bool _set=false):
      desc(_desc), type(_type), settable(_set) {}
    Property(const PropertyDescription& _desc, Value::Type _type,
             const string& _xpath, bool _set=false):
      desc(_desc), type(_type), xpath(_xpath), settable(_set) {}
    Property(const PropertyDescription& _desc, Value::Type _type,
             const string& _xpath, const set<string>& _options, bool _set=false):
      desc(_desc), type(_type), xpath(_xpath), options(_options),
        settable(_set) {}
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
public:
  // Control value setting parameters
  struct SetParams
  {
    Value v;                // Value to set (increment)
    bool increment{false};  // True if incremental

    // Default for maps
    SetParams() {}

    // Simple set
    SetParams(const Value& _v): v(_v) {}

    // Incremental set
    SetParams(const Value& _v, bool _inc): v(_v), increment(_inc) {}
  };

protected:
  // Param setting helpers
  static void update_prop(double &prop, const SetParams& sp)
  { if (sp.increment) prop+=sp.v.d; else prop = sp.v.d; }
  static void update_prop_int(int &prop, const SetParams& sp)
  { int i=static_cast<int>(sp.v.d);
    if (sp.increment) prop+=i; else prop = i; }
  static void update_prop(bool &prop, const SetParams& sp)
  { if (sp.increment) prop = !prop; else prop = !!sp.v.d; }

public:
  const Module *module{nullptr};
  string id;
  Graph *graph{nullptr};
  Engine *engine{nullptr};
  Element *next_element{nullptr};
  list<Element *> downstreams; // All data and control connections, for toposort

  // Default constructor for virtual inheritance
  Element() {}

  // Basic construction with XML - just sets ID
  // Extend this to read basic config which doesn't take much time or
  // require registry - i.e. just reading basic config attributes
  Element(const Module *_module, const XML::Element& config):
    module(_module), id(config["id"]) {}
  Element(const XML::Element& config): id(config["id"]) {} // for test
  Element(const string& _id): id(_id) {}  // for test

  // Configure with XML config
  // Implement this for configuration which could take time, or needs
  // Engine registries to create sub-graphs / access services
  // Throw a runtime_error if configuration fails
  virtual void configure(const File::Directory& /*base_dir*/,
                         const XML::Element& /*config*/) {}

  // Connect to other elements in the graph, for cases where the normal
  // graph connection isn't sufficient.  Called when the graph is already
  // loaded and all elements configured, and after normal connection
  virtual void connect() {}

  // Notify that this element is the target of another element
  virtual void notify_target_of(Element *) {}

  // Set a control value
  virtual void set_property(const string& property, const SetParams&)
  { throw runtime_error("No such property "+property+" on element "+id); }

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
  virtual void tick(const TickData&) {};

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
class Generator: virtual public Element
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

  // Set the acceptor
  virtual void attach(Acceptor *_acceptor) { acceptor = _acceptor; }

  // Send data down
  void send(DataPtr data) { if (acceptor) acceptor->accept(data); }
  // Sugar to allow direct send of 'new Data' in sources
  void send(Data *data) { send(DataPtr(data)); }
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
class Sink: virtual public Element, public Acceptor
{
 public:
  using Element::Element;
};

//==========================================================================
// Control - sets one or more properties on a target Element
class Control: virtual public Element
{
 public:
  struct Property
  {
    string name;  // Target property name
    Value::Type type{Value::Type::invalid};
    Property() {}
    Property(const string& _name, Value::Type _type): name(_name), type(_type) {}
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
  Control(const Module *_module, const XML::Element& _config,
          bool targets_are_optional = false);

  // Get targets
  const map<string, Target>& get_targets() { return targets; }

  // Attach to a target element
  void attach_target(const string& id, Element *element);

  // Send a value to the target using only (first) property
  void send(const SetParams& sp);

  // Send a named value to the target
  // name is our name for it
  void send(const string& name, const SetParams& sp);
};

//==========================================================================
// Dataflow graph structure
class Graph
{
  Engine& engine;
  MT::RWMutex mutex;
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
  Engine& get_engine() { return engine; }

  //------------------------------------------------------------------------
  // Configure with XML, with a base directory for files
  // Throws a runtime_error if configuration fails
  void configure(const File::Directory& base_dir, const XML::Element& config);

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
  // Shutdown all elements
  void shutdown();
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

private:
  struct ModuleInfo
  {
    const Module *module;
    const Factory *factory;
    ModuleInfo() {}
    ModuleInfo(const Module *_module, const Factory *_factory):
      module(_module), factory(_factory) {}
  };
  map<string, ModuleInfo> modules;

public:
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
  MT::Mutex graph_mutex;
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
  // Tick the graph
  void tick(Time::Stamp t);

  //------------------------------------------------------------------------
  // Shut down the graph
  void shutdown();
};


//==========================================================================
}} //namespaces
#endif // !__VG_DATAFLOW_H
