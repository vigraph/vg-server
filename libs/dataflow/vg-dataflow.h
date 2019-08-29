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
#include "ot-mt.h"
#include "ot-init.h"
#include "ot-text.h"
#include "ot-time.h"
#include "ot-file.h"
#include "ot-log.h"
#include "ot-json.h"

namespace ViGraph { namespace Dataflow {

// Make our lives easier without polluting anyone else
using namespace std;
using namespace ObTools;
using namespace ViGraph;

const auto default_frequency = 25.0;
const auto default_tick_interval = Time::Duration{1.0 / default_frequency};
const auto default_sample_rate = default_frequency;

// Typedefs
typedef double timestamp_t; // Relative timestamp

//==========================================================================
// Tick data - data that is passed for each tick
struct TickData
{
  Time::Stamp timestamp;
  double sample_rate = 0;
  unsigned long nsamples = 0;

  TickData(const Time::Stamp& _timestamp, double _sample_rate,
           unsigned long _nsamples):
    timestamp{_timestamp}, sample_rate{_sample_rate}, nsamples{_nsamples}
  {}
};

// forward declarations
class Engine;
class Graph;
class Element;

//==========================================================================
// Visitor class
class Visitor
{
public:
  virtual void visit(Engine& engine) = 0;
  virtual unique_ptr<Visitor> getSubGraphVisitor() = 0;
  virtual void visit(Graph& graph) = 0;
  virtual unique_ptr<Visitor> getSubElementVisitor(const string& id) = 0;
  virtual void visit(Element& element) = 0;
  virtual ~Visitor() {}
};

//==========================================================================
// Element setting interface
class ElementSetting
{
public:
  virtual ~ElementSetting() {}
};

//==========================================================================
// Element setting template
template<typename T>
class Setting: public ElementSetting
{
private:
  T value;

public:
  Setting(const T& _value = T{}): value{_value} {}

  void set(const T& _value) { value = _value; }
  T get() const { return value; }

  void accept(Visitor& visitor)
  {
    visitor.visit(*this);
  }
};

//==========================================================================
// Element input interface
class ElementInput: public ElementSetting
{
public:
  virtual bool ready() const = 0;
  virtual void reset() = 0;
};

//==========================================================================
// Element output interface
class ElementOutput
{
public:
  struct Connection
  {
    Element *element = nullptr;
    ElementInput *input = nullptr;
    Connection() {}
    Connection(Element *_element, ElementInput *_input):
      element{_element}, input{_input}
    {}
  };
  virtual bool connect(const Connection&) = 0;
  virtual vector<Connection> get_connections() const = 0;
};

template<typename T> class Output;

//--------------------------------------------------------------------------
// Combine output data for input


//==========================================================================
// Element input template
template<typename T>
class Input: public Setting<T>, public virtual ElementInput
{
private:
  friend class Output<T>;
  struct Data
  {
    vector<T> data;
    bool ready = false;
  };
  map<Output<T> *, Data> input_data;
  bool combined = false;


  // Combine for types which addition is valid for
  template<typename U = T, class = decltype(declval<U>() + declval<U>())>
  void combine(decltype(input_data.begin()) it)
  {
    for (auto i = input_data.begin(); i != input_data.end(); ++i)
    {
      if (i == it)
        continue;
      auto c = it->second.data.begin();
      for (const auto& b: i->second.data)
      {
        *c += b;
        if (++c == it->second.data.end())
          break;
      }
    }
    combined = true;
  }

  // Combine for types which can't be added
  void combine(decltype(input_data.begin()))
  {
    // Just use the first connection
    combined = true;
  }

public:
  using Setting<T>::Setting;

  bool ready() const override
  {
    for (const auto& i: input_data)
    {
      if (!i.second.ready)
        return false;
    }
    return true;
  }

  bool connected() const
  {
    return !input_data.empty();
  }

  const vector<T>& get_buffer()
  {
    static auto empty = vector<T>{};
    if (input_data.empty())
      return empty;

    auto it = input_data.begin();
    if (!combined && input_data.size() > 1)
      combine(it);

    return it->second.data;
  }

  void reset() override
  {
    if (!input_data.empty())
    {
      auto it = input_data.begin();

      // Ensure combined
      if (!combined && input_data.size() > 1)
        combine(it);

      // Store last value
      this->set(it->second.data.back());
    }

    for (auto& i: input_data)
    {
      i.second.data.clear();
      i.second.ready = false;
    }
  }

  ~Input()
  {
    while (!input_data.empty())
      input_data.begin()->first->disconnect(*this);
  }
};

//--------------------------------------------------------------------------
// ostream output for settings
template<typename T>
ostream& operator<<(ostream& os, const Setting<T>& s)
{
  os << s.get();
  return os;
}

//==========================================================================
// Element output template
template<typename T>
class Output: public ElementOutput
{
private:
  struct OutputData
  {
    Element *element = nullptr;
    typename Input<T>::Data *input = nullptr;
    OutputData() {}
    OutputData(Element *_element, typename Input<T>::Data *_input):
      element{_element}, input{_input}
    {}
  };
  map<Input<T> *, OutputData> output_data;
  Input<T> *primary_data = nullptr;

public:
  void connect(Element *element, Input<T>& to)
  {
    output_data[&to] = {element, &to.input_data[this]};
    if (!primary_data)
      primary_data = &to;
  }

  bool connect(const Connection& connection) override
  {
    auto ito = dynamic_cast<Input<T> *>(connection.input);
    if (!ito)
      return false;
    connect(connection.element, *ito);
    return true;
  }

  void disconnect(Input<T>& to)
  {
    to.input_data.erase(this);
    output_data.erase(&to);
    if (primary_data == &to)
    {
      if (output_data.empty())
        primary_data = nullptr;
      else
        primary_data = output_data.begin()->first;
    }
  }

  bool connected() const
  {
    return primary_data;
  }

  struct Buffer
  {
    Output<T> *out = nullptr;
    vector<T>& data;
    Buffer(Output<T> * _out, vector<T>& _data): out{_out}, data{_data} {}
    ~Buffer() { if(out) out->complete(); }
  };
  Buffer get_buffer()
  {
    static auto empty = vector<T>{};
    static auto empty_buffer = Buffer{nullptr, empty};
    if (!primary_data)
      return empty_buffer;
    return Buffer{this, output_data[primary_data].input->data};
  }

  vector<Connection> get_connections() const override
  {
    auto result = vector<Connection>{};
    for (const auto& od: output_data)
      result.emplace_back(od.second.element, od.first);
    return result;
  }

  void complete()
  {
    if (!output_data.empty())
    {
      const auto& b = output_data[primary_data];

      for (auto& o: output_data)
      {
        if (o.first != primary_data)
          o.second.input->data = b.input->data;
        o.second.input->ready = true;
      }
    }
  }

  ~Output()
  {
    for (auto& od: output_data)
      od.first->input_data.erase(this);
  }
};

//==========================================================================
// Module metadata

template<typename T> inline string get_module_type();
template<typename T>
inline void set_from_json(T& value, const JSON::Value& json);
template<typename T>
inline JSON::Value get_as_json(const T& value);


template<>
inline string get_module_type<double>() { return "number"; }

template<>
inline void set_from_json(double& value, const JSON::Value& json)
{
  if (json.type == JSON::Value::NUMBER)
    value = json.f;
  else
    value = json.n;
}

template<>
inline JSON::Value get_as_json(const double& value)
{
  return {value};
}

template<>
inline string get_module_type<string>() { return "text"; }

template<>
inline void set_from_json(string& value, const JSON::Value& json)
{
  value = json.s;
}

template<>
inline JSON::Value get_as_json(const string& value)
{
  return {value};
}

struct Module
{
  string id;
  string name;
  string section;

  string type() const
  {
    return section + ":" + id;
  }

  // settings
  class Setting
  {
  private:
    class MemberFunctor
    {
    public:
      virtual ElementSetting& get(Element& b) const = 0;
      virtual JSON::Value get_json(Element& b) const = 0;
      virtual void set_json(Element& b, const JSON::Value& json) const = 0;
      virtual ~MemberFunctor() {}
    };
    template<typename T>
    class MemberFunctorImpl: public MemberFunctor
    {
    private:
      Dataflow::Setting<T> Element::* member_pointer = nullptr;

    public:
      template<typename C>
      MemberFunctorImpl(Dataflow::Setting<T> C::* _member_pointer):
        member_pointer{static_cast<Dataflow::Setting<T> Element::*>(
                       _member_pointer)}
      {}

      ElementSetting& get(Element& b) const override
      {
        return b.*member_pointer;
      }

      JSON::Value get_json(Element& b) const override
      {
        return get_as_json((b.*member_pointer).get());
      }

      void set_json(Element& b, const JSON::Value& json) const override
      {
        auto v = T{};
        set_from_json(v, json);
        (b.*member_pointer).set(v);
      }
    };
    shared_ptr<MemberFunctor> member_functor;

  public:
    const string type;

    template<typename T, typename C>
    Setting(Dataflow::Setting<T> C::* i):
      member_functor{new MemberFunctorImpl<T>{i}}, type{get_module_type<T>()}
    {}

    ElementSetting& get(Element& b) const
    {
      return member_functor->get(b);
    }

    JSON::Value get_json(Element& b) const
    {
      return member_functor->get_json(b);
    }

    void set_json(Element& b, const JSON::Value& json) const
    {
      member_functor->set_json(b, json);
    }
  };
  map<string, Setting> settings;

  class Input
  {
  private:
    class MemberFunctor
    {
    public:
      virtual ElementInput& get(Element& b) const = 0;
      virtual JSON::Value get_json(Element& b) const = 0;
      virtual void set_json(Element& b, const JSON::Value& json) const = 0;
      virtual ~MemberFunctor() {}
    };
    template<typename T>
    class MemberFunctorImpl: public MemberFunctor
    {
    private:
      Dataflow::Input<T> Element::* member_pointer = nullptr;

    public:
      template<typename C>
      MemberFunctorImpl(Dataflow::Input<T> C::* _member_pointer):
        member_pointer{static_cast<Dataflow::Input<T> Element::*>(
                      _member_pointer)}
      {}

      ElementInput& get(Element& b) const override
      {
        return b.*member_pointer;
      }

      JSON::Value get_json(Element& b) const override
      {
        return get_as_json((b.*member_pointer).get());
      }

      void set_json(Element& b, const JSON::Value& json) const override
      {
        auto v = T{};
        set_from_json(v, json);
        (b.*member_pointer).set(v);
      }
    };
    shared_ptr<MemberFunctor> member_functor;

  public:
    const string type;

    template<typename T, typename C>
    Input(Dataflow::Input<T> C::* i):
      member_functor{new MemberFunctorImpl<T>{i}}, type{get_module_type<T>()}
    {}

    ElementInput& get(Element& b) const
    {
      return member_functor->get(b);
    }

    JSON::Value get_json(Element& b) const
    {
      return member_functor->get_json(b);
    }

    void set_json(Element& b, const JSON::Value& json) const
    {
      member_functor->set_json(b, json);
    }
  };
  map<string, Input> inputs;

  class Output
  {
  private:
    class MemberFunctor
    {
    public:
      virtual ElementOutput& get(Element& b) const = 0;
      virtual ~MemberFunctor() {}
    };
    template<typename T>
    class MemberFunctorImpl: public MemberFunctor
    {
    private:
      Dataflow::Output<T> Element::* member_pointer = nullptr;

    public:
      template<typename C>
      MemberFunctorImpl(Dataflow::Output<T> C::* _member_pointer):
        member_pointer{static_cast<Dataflow::Output<T> Element::*>(
                      _member_pointer)}
      {}

      ElementOutput& get(Element& b) const override
      {
        return b.*member_pointer;
      }
    };
    shared_ptr<MemberFunctor> member_functor;

  public:
    const string type;

    template<typename T, typename C>
    Output(Dataflow::Output<T> C::* o):
      member_functor{new MemberFunctorImpl<T>{o}}, type{get_module_type<T>()}
    {}

    ElementOutput& get(Element& b) const
    {
      return member_functor->get(b);
    }
  };
  map<string, Output> outputs;

  Module(const string& _id, const string& _name, const string& _section,
         const map<string, Setting>& _settings,
         const map<string, Input>& _inputs,
         const map<string, Output>& _outputs):
    id{_id}, name{_name}, section{_section}, settings{_settings},
    inputs{_inputs}, outputs{_outputs}
  {}

  string get_input_id(Element& element, ElementInput& input) const
  {
    for (const auto& i: inputs)
      if (&i.second.get(element) == &input)
        return i.first;
    return "[invalid]";
  }
};

//==========================================================================
// Graph element - just has an ID and a parent graph
class Element
{
private:
  set<ElementInput *> inputs;

  template<typename... Ss, size_t... Sc, typename... Is, size_t... Ic,
           typename... Os, size_t... Oc, typename F>
  void sample_iterate_impl(unsigned int count,
                           const tuple<Ss...>& ss, index_sequence<Sc...>,
                           const tuple<Is...>& is, index_sequence<Ic...>,
                           const tuple<Os...>& os, index_sequence<Oc...>,
                           const F& f)
  {
    auto settings = make_tuple(get<Sc>(ss).get()...);
    auto inputs = make_tuple(get<Ic>(is).get_buffer()...);
    auto outputs = make_tuple(get<Oc>(os).get_buffer()...);
    // Resize all outputs to wanted size
    int dummy[] = {0, (void(get<Oc>(outputs).data.resize(count)), 0)...};
    (void)dummy;
    for (auto i = 0u; i < count; ++i)
    {
      f(get<Sc>(settings)...,
        (get<Ic>(inputs).size() > i ? get<Ic>(inputs)[i]
                                    : get<Ic>(is).get())...,
        get<Oc>(outputs).data[i]...
       );
    }
  }

protected:
  template<typename... Ss, typename... Is, typename... Os, typename F>
  void sample_iterate(const unsigned count, const tuple<Ss...>& ss,
                      const tuple<Is...>& is, const tuple<Os...>& os,
                      const F& f)
  {
    sample_iterate_impl(count,
                        ss, index_sequence_for<Ss...>{},
                        is, index_sequence_for<Is...>{},
                        os, index_sequence_for<Os...>{},
                        f);
  }

public:
  const Module& module;
  string id;
  Graph *graph{nullptr};
  Engine *engine{nullptr};

  // Basic construction
  // Extend this to read basic config which doesn't take much time or
  // require registry - i.e. just reading basic config attributes
  Element(const Module& _module):
    module{_module} {}

  // Setup after automatic configuration
  virtual void setup() {}

  // Delete item from JSON
  // path is a path/to/leaf
  // Fails here, override in container elements
  virtual void delete_item(const string& path)
  { throw runtime_error("Can't delete subelement "+path+
                        " in leaf element "+id); }

  // Connect an element
  bool connect(const string& out_name, Element& b, const string &in_name);

  // Set a Setting/Input
  template<typename T>
  Element& set(const string& setting, const T& value)
  {
    auto sit = module.settings.find(setting);
    if (sit != module.settings.end())
    {
      auto s = dynamic_cast<Setting<T> *>(&(sit->second.get(*this)));
      if (s)
        s->set(value);
    }
    else
    {
      auto iit = module.inputs.find(setting);
      if (iit != module.inputs.end())
      {
        auto i = dynamic_cast<Input<T> *>(&(iit->second.get(*this)));
        if (i)
          i->set(value);
      }
    }
    return *this;
  }

  // Update after setting a setting
  virtual void update() {}

  // Notify of parent graph being enabled - register for keys etc.
  virtual void enable() {}

  // Notify of parent graph being disabled - de-register for keys etc.
  virtual void disable() {}

  // Is ready to process tick
  bool ready() const;

  // Tick
  virtual void tick(const TickData& /*tick data*/) {}

  // Prepare for a tick
  void reset();

  // Accept a visitor
  void accept(Visitor& visitor);

  // Clean shutdown
  virtual void shutdown() {}

  // Virtual destructor
  virtual ~Element() {}
};

class Generator;  // forward

//==========================================================================
// Dataflow graph structure
class Graph
{
private:
  Engine& engine;
  mutable MT::RWMutex mutex;
  map<string, shared_ptr<Element> > elements;   // By ID
  Graph *parent{nullptr};
  double sample_rate = 0;

  // Internal
  Element *create_element(const string& type, const string& id);

public:
  //------------------------------------------------------------------------
  // Constructor
  Graph(Engine& _engine, Graph *_parent=nullptr):
    engine(_engine), parent(_parent) {}

  //------------------------------------------------------------------------
  // Get engine
  Engine& get_engine() const
  { return engine; }

  //------------------------------------------------------------------------
  // Add an element to the graph
  void add_element(const string& type, const string& id);

  //------------------------------------------------------------------------
  // Get all elements (for inspection)
  const map<string, shared_ptr<Element> >& get_elements() const
  { return elements; }

  //------------------------------------------------------------------------
  // Add an element to the graph (testing)
  void add(Element *el);

  //------------------------------------------------------------------------
  // Final setup for elements and calculate topology
  void setup();

  //------------------------------------------------------------------------
  // Set sample rate
  void set_sample_rate(double sr) { sample_rate = sr; }

  //------------------------------------------------------------------------
  // Tick all elements
  void tick(const TickData& td);

  //------------------------------------------------------------------------
  // Get a particular element by ID
  Element *get_element(const string& id);

  //------------------------------------------------------------------------
  // Get the nearest particular element by section and type, looking upwards
  // in ancestors
  shared_ptr<Element> get_nearest_element(const string& section,
                                          const string& type);

  //------------------------------------------------------------------------
  // Get type-checked nearest service element (can be nullptr if doesn't
  // exist or wrong type)
  template <class T> shared_ptr<T> find_service(const string& section,
                                                const string& type)
  {
    auto el = get_nearest_element(section, type);
    if (!el) return {};
    auto t = dynamic_pointer_cast<T>(el);
    if (!t) return {};
    return t;
  }

  //------------------------------------------------------------------------
  // Delete an item (from REST)
  // path is a path/to/leaf
  void delete_item(const string& path);

  //------------------------------------------------------------------------
  // Clear all elements
  void clear_elements()
  {
    elements.clear();
  }

  //------------------------------------------------------------------------
  // Accept a visitor
  void accept(Visitor& visitor);

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
// Generic singleton service - no inputs or outputs
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
    virtual Element *create(const Module& module) const = 0;
    virtual ~Factory() {}
  };

  // Template for factories creating with { new Type }
  template<class E> struct NewFactory: public Factory
  {
  public:
    Element *create(const Module& module) const
    { return new E(module); }
  };

  struct ModuleInfo
  {
    const Module *module = nullptr;
    const Factory *factory = nullptr;
    ModuleInfo() {}
    ModuleInfo(const Module& _module, const Factory& _factory):
      module(&_module), factory(&_factory) {}
  };

  struct Section
  {
    map<string, ModuleInfo> modules;
  };

  map<string, Section> sections;

  //------------------------------------------------------------------------
  // Constructor
  Registry() {}

  //------------------------------------------------------------------------
  // Register a module with its factory
  void add(const Module& m, const Factory& f)
  { sections[m.section].modules[m.id] = ModuleInfo(m, f); }

  //------------------------------------------------------------------------
  // Create an object by module and config
  // Returns the object, or 0 if no factories available or create fails
  Element *create(const string& section, const string& id)
  {
    const auto sp = sections.find(section);
    if (sp == sections.end()) return 0;

    const auto mp = sp->second.modules.find(id);
    if (mp == sp->second.modules.end()) return 0;

    const auto& mi = mp->second;
    return mi.factory->create(*mi.module);
  }
};

//==========================================================================
// Engine class - wrapper containing Graph tree and Element registry
class Engine
{
  // Graph structure
  mutable MT::RWMutex graph_mutex;
  unique_ptr<Dataflow::Graph> graph;
  Time::Duration tick_interval = default_tick_interval;
  double sample_rate = default_sample_rate;
  Time::Stamp start_time;
  uint64_t tick_number{0};
  list<string> default_sections;  // Note: ordered

 public:
  Registry element_registry;

  //------------------------------------------------------------------------
  // Constructor
  Engine(): graph(new Graph(*this)) {}

  //------------------------------------------------------------------------
  // Add a default section - use to auto-prefix unqualified element names
  void add_default_section(const string& s)
  { default_sections.push_back(s); }

  //------------------------------------------------------------------------
  // Set/get the tick interval
  void set_tick_interval(const Time::Duration& d) { tick_interval = d; }
  Time::Duration get_tick_interval() const { return tick_interval; }

  //------------------------------------------------------------------------
  // Set/get the sample rate
  void set_sample_rate(double sr)
  { sample_rate = sr; graph->set_sample_rate(sr); }
  double get_sample_rate() const { return sample_rate; }

  //------------------------------------------------------------------------
  // Get the graph (for testing only)
  Dataflow::Graph& get_graph() { return *graph; }

  //------------------------------------------------------------------------
  // Create an element with the given name - may be section:id or just id,
  // which is looked up in default namespaces
  Element *create(const string& name);

  //------------------------------------------------------------------------
  // Delete an item (from REST)
  // path is a path/to/leaf
  void delete_item(const string& path);

  //------------------------------------------------------------------------
  // Tick the graph
  void tick(Time::Stamp t);

  //------------------------------------------------------------------------
  // Accept a visitor
  void accept(Visitor& visitor, bool write);

  //------------------------------------------------------------------------
  // Shut down the graph
  void shutdown();
};

//==========================================================================
}} //namespaces
#endif // !__VG_DATAFLOW_H
