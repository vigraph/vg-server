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

const auto namespace_separator = '/';
const auto default_frequency = 25.0;
const auto default_tick_interval = Time::Duration{1.0 / default_frequency};
const auto default_sample_rate = default_frequency;

// Typedefs
typedef double timestamp_t; // Relative timestamp

//==========================================================================
// Tick data - data that is passed for each tick
struct TickData
{
  timestamp_t start = 0;
  timestamp_t end = 0;

  TickData(timestamp_t _start, timestamp_t _end):
    start{_start}, end{_end}
  {}

  timestamp_t sample_duration(double sample_rate) const
  {
    return 1.0 / sample_rate;
  }

  unsigned long samples_at(double sample_rate, timestamp_t t) const
  {
    return t * sample_rate;
  }

  timestamp_t first_sample_at(double sample_rate) const
  {
    const auto sd = sample_duration(sample_rate);
    const auto offset = fmod(start, sd);
    if (fabs(offset) > 1e-15)
      return start - offset + sd;
    else
      return start;
  }

  unsigned long samples_in_tick(double sample_rate) const
  {
    return samples_at(sample_rate, end) - samples_at(sample_rate, start);
  }
};

// forward declarations
class Engine;
class Graph;
class Clone;
class Element;
class ElementSetting;
class ElementInput;
class ElementOutput;
class GraphElement;
class ReadVisitor;
class WriteVisitor;

//==========================================================================
// Path class
class Path
{
public:
  enum class PartType
  {
    none,
    element,
    attribute,
  };

private:
  struct Part
  {
    string name;
    PartType type;
    Part(const string& _name, PartType _type):
      name{_name}, type{_type}
    {}
  };
  vector<Part> parts;

public:
  Path(const string& path)
  {
    if (!path.empty())
    {
      auto _parts = Text::split(path, '/');
      for (const auto p: _parts)
      {
        if (p.empty())
          continue;
        else if (p[0] == '@')
          parts.emplace_back(p.substr(1, p.size() - 1), PartType::attribute);
        else
          parts.emplace_back(p, PartType::element);
      }
    }
  }

  bool reached(decltype(parts.size()) index) const
  {
    return empty() || index >= parts.size();
  }

  auto get(decltype(parts.size()) index) const
  {
    return parts[index];
  }

  string name(decltype(parts.size()) index) const
  {
    if (index < parts.size())
      return parts[index].name;
    else
      return "";
  }

  PartType type(decltype(parts.size()) index) const
  {
    if (index < parts.size())
      return parts[index].type;
    else
      return PartType::none;
  }

  auto size() const
  {
    return parts.size();
  }

  bool empty() const
  {
    return parts.empty();
  }
};

//==========================================================================
// Member wrappers
class SettingMember
{
public:
  virtual string get_type() const = 0;
  virtual const ElementSetting& get(const GraphElement& b) const = 0;
  virtual ElementSetting& get(GraphElement& b) const = 0;
  virtual JSON::Value get_json(const GraphElement& b) const = 0;
  virtual void set_json(GraphElement& b, const JSON::Value& json) const = 0;
  virtual void accept(ReadVisitor& visitor,
                      const Path& path, unsigned path_index,
                      const GraphElement& element) const = 0;
  virtual void accept(WriteVisitor& visitor,
                      const Path& path, unsigned path_index,
                      GraphElement& element) const = 0;
  virtual ~SettingMember() {}
};

class InputMember
{
public:
  virtual string get_type() const = 0;
  virtual const ElementInput& get(const GraphElement& b) const = 0;
  virtual ElementInput& get(GraphElement& b) const = 0;
  virtual JSON::Value get_json(const GraphElement& b) const = 0;
  virtual void set_json(GraphElement& b, const JSON::Value& json) const = 0;
  virtual void accept(ReadVisitor& visitor,
                      const Path& path, unsigned path_index,
                      const GraphElement& element) const = 0;
  virtual void accept(WriteVisitor& visitor,
                      const Path& path, unsigned path_index,
                      GraphElement& element) const = 0;
  virtual double get_sample_rate(const GraphElement& b) const = 0;
  virtual ~InputMember() {}
};

class OutputMember
{
public:
  virtual string get_type() const = 0;
  virtual const ElementOutput& get(const GraphElement& b) const = 0;
  virtual ElementOutput& get(GraphElement& b) const = 0;
  virtual void accept(ReadVisitor& visitor,
                      const Path& path, unsigned path_index,
                      const GraphElement& element) const = 0;
  virtual void accept(WriteVisitor& visitor,
                      const Path& path, unsigned path_index,
                      GraphElement& element) const = 0;
  virtual double get_sample_rate(const GraphElement& b) const = 0;
  virtual ~OutputMember() {}
};

//==========================================================================
// Read Visitor class
class ReadVisitor
{
public:
  virtual void visit(const Engine& engine,
                     const Path& path, unsigned path_index) = 0;
  virtual unique_ptr<ReadVisitor> get_root_graph_visitor(
                     const Path& path, unsigned path_index) = 0;
  virtual void visit(const Graph& graph,
                     const Path& path, unsigned path_index) = 0;
  virtual void visit(const Clone& clone,
                     const Path& path, unsigned path_index) = 0;
  virtual unique_ptr<ReadVisitor> get_sub_element_visitor(
                                                    const Graph &graph,
                                                    const string& id,
                                                    const Path& path,
                                                    unsigned path_index) = 0;
  virtual void visit(const Element& element,
                     const Path& path, unsigned path_index) = 0;
  virtual unique_ptr<ReadVisitor> get_element_setting_visitor(
                                              const GraphElement& element,
                                              const string& id,
                                              const Path& path,
                                              unsigned path_index) = 0;
  virtual void visit(const GraphElement& element, const SettingMember& setting,
                     const Path& path, unsigned path_index) = 0;
  virtual unique_ptr<ReadVisitor> get_element_input_visitor(
                                              const GraphElement& element,
                                              const string& id,
                                              const Path& path,
                                              unsigned path_index) = 0;
  virtual void visit(const GraphElement& element, const InputMember& input,
                     const Path& path, unsigned path_index) = 0;
  virtual unique_ptr<ReadVisitor> get_element_output_visitor(
                                              const GraphElement& element,
                                              const string& id,
                                              const Path& path,
                                              unsigned path_index) = 0;
  virtual void visit(const GraphElement& element, const OutputMember& output,
                     const Path& path, unsigned path_index) = 0;
  virtual void visit_graph_input_or_output(const Graph& graph,
                                           const string& id,
                                           const Path& path,
                                           unsigned path_index) = 0;

  virtual ~ReadVisitor() {}
};

//==========================================================================
// Write Visitor class
class WriteVisitor
{
public:
  virtual void visit(Engine& engine,
                     const Path& path, unsigned path_index) = 0;
  virtual unique_ptr<WriteVisitor> get_root_graph_visitor(
                     const Path& path, unsigned path_index) = 0;
  virtual bool visit(Graph& graph,
                     const Path& path, unsigned path_index) = 0;
  virtual bool visit(Clone& clone,
                     const Path& path, unsigned path_index) = 0;
  virtual unique_ptr<WriteVisitor> get_sub_element_visitor(
                                                    Graph &graph,
                                                    const string& id,
                                                    const Path& path,
                                                    unsigned path_index) = 0;
  virtual unique_ptr<WriteVisitor> get_sub_clone_visitor(
                                                    Clone& clone,
                                                    const string& id,
                                                    const Path& path,
                                                    unsigned path_index) = 0;
  virtual bool visit(Element& element,
                     const Path& path, unsigned path_index) = 0;
  virtual unique_ptr<WriteVisitor> get_element_setting_visitor(
                                              GraphElement& element,
                                              const string& id,
                                              const Path& path,
                                              unsigned path_index) = 0;
  virtual void visit(GraphElement& element, const SettingMember& setting,
                     const Path& path, unsigned path_index) = 0;
  virtual unique_ptr<WriteVisitor> get_element_input_visitor(
                                              GraphElement& element,
                                              const string& id,
                                              const Path& path,
                                              unsigned path_index) = 0;
  virtual void visit(GraphElement& element, const InputMember& input,
                     const Path& path, unsigned path_index) = 0;
  virtual unique_ptr<WriteVisitor> get_element_output_visitor(
                                              GraphElement& element,
                                              const string& id,
                                              const Path& path,
                                              unsigned path_index) = 0;
  virtual void visit(GraphElement& element, const OutputMember& output,
                     const Path& path, unsigned path_index) = 0;
  virtual void visit_graph_input_or_output(Graph& graph,
                                           const string& id,
                                           const Path& path,
                                           unsigned path_index) = 0;

  virtual ~WriteVisitor() {}
};

//==========================================================================
// Element setting interface
class ElementSetting
{
public:
  virtual void set_from(const ElementSetting& setting) = 0;

  // Accept visitors
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
  Setting<T>& operator=(const T& _value) { value = _value; return *this; }

  T get() const { return value; }
  operator T() const { return value; }
  bool operator==(const T& o) const { return value == o; }
  bool operator!=(const T& o) const { return value != o; }

  void set_from(const ElementSetting& setting) override
  {
    const auto s = dynamic_cast<const Setting<T> *>(&setting);
    if (s)
      set(s->get());
  }
};

//==========================================================================
// Element input interface
class ElementInput: public ElementSetting
{
public:
  virtual double get_sample_rate() const = 0;
  virtual void set_sample_rate(double rate) = 0;
  struct Connection
  {
    GraphElement *element = nullptr;
    ElementOutput *output = nullptr;
    Connection() {}
    Connection(GraphElement *_element, ElementOutput *_output):
      element{_element}, output{_output}
    {}
  };
  virtual bool ready() const = 0;
  virtual void reset() = 0;
  virtual vector<Connection> get_connections() const = 0;
  virtual void disconnect() = 0;
};

//==========================================================================
// Element output interface
class ElementOutput
{
public:
  virtual double get_sample_rate() const = 0;
  virtual void set_sample_rate(double rate) = 0;
  struct Connection
  {
    GraphElement *element = nullptr;
    ElementInput *input = nullptr;
    Connection() {}
    Connection(GraphElement *_element, ElementInput *_input):
      element{_element}, input{_input}
    {}
  };
  virtual bool connect(GraphElement *, const Connection&) = 0;
  virtual vector<Connection> get_connections() const = 0;
  virtual void disconnect() = 0;

  virtual ~ElementOutput() {}
};

template<typename T> class Output;

//==========================================================================
// Element input template
template<typename T>
class Input: public Setting<T>, public virtual ElementInput
{
private:
  friend class Output<T>;

  double sample_rate = 0.0;
  struct Data
  {
    GraphElement *element = nullptr;
    vector<T> data;
    bool ready = false;
  };
  map<Output<T> *, Data> input_data;
  bool combined = false;


  // Combine for types which addition is valid for
  // Second arg is for disambiguation, always pass true
  template<typename U = T, class = decltype(declval<U>() + declval<U>())>
  void combine(decltype(input_data.begin()) it, bool)
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
  // Second arg is for disambiguation, always pass true
  template<typename U = T>
  void combine(decltype(input_data.begin()), int)
  {
    // Just use the first connection
    combined = true;
  }

public:
  using Setting<T>::Setting;

  void set_from(const ElementSetting& setting) override
  {
    Setting<T>::set_from(setting);
  }

  double get_sample_rate() const override
  {
    return sample_rate;
  }

  void set_sample_rate(double rate) override
  {
    sample_rate = rate;
    for (auto& i: input_data)
    {
      i.first->set_sample_rate(rate);
      i.second.element->update_sample_rate();
    }
  }

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
      combine(it, true);

    return it->second.data;
  }

  void reset() override
  {
    if (!input_data.empty())
    {
      auto it = input_data.begin();

      // Ensure combined
      if (!combined && input_data.size() > 1)
        combine(it, true);

      // Store last value
      if (!it->second.data.empty())
        this->set(it->second.data.back());
    }
    combined = false;

    for (auto& i: input_data)
    {
      i.second.data.clear();
      i.second.ready = false;
    }
  }

  vector<Connection> get_connections() const override
  {
    auto result = vector<Connection>{};
    for (const auto& id: input_data)
      result.emplace_back(id.second.element, id.first);
    return result;
  }

  // Disconnect everything
  void disconnect() override
  {
    while (!input_data.empty())
      input_data.begin()->first->disconnect(*this);
  }

  ~Input()
  {
    disconnect();
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
  double sample_rate = 0.0;

  struct OutputData
  {
    GraphElement *element = nullptr;
    typename Input<T>::Data *input = nullptr;
    OutputData() {}
    OutputData(GraphElement *_element, typename Input<T>::Data *_input):
      element{_element}, input{_input}
    {}
  };
  map<Input<T> *, OutputData> output_data;
  Input<T> *primary_data = nullptr;

public:
  void connect(GraphElement *from_element,
               GraphElement *to_element, Input<T>& to)
  {
    auto& id = to.input_data[this];
    id.element = from_element;
    output_data[&to] = {to_element, &id};
    if (!primary_data)
      primary_data = &to;
  }

  double get_sample_rate() const override
  {
    return sample_rate;
  }

  void set_sample_rate(double rate) override
  {
    sample_rate = rate;
  }

  bool connect(GraphElement *element, const Connection& connection) override
  {
    auto ito = dynamic_cast<Input<T> *>(connection.input);
    if (!ito)
      return false;
    connect(element, connection.element, *ito);
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

  // Disconnect everything
  void disconnect() override
  {
    primary_data = nullptr;
    for (const auto& od: output_data)
      od.first->input_data.erase(this);
    output_data.clear();
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
    ~Buffer() { if (out) out->complete(); }
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

// Interface
class Module
{
public:
  virtual string get_id() const = 0;
  virtual string get_name() const = 0;
  virtual string get_section() const = 0;
  virtual string get_full_type() const { return get_section() +
                                                namespace_separator +
                                                get_id(); }
  virtual bool is_dynamic() const { return false; }
  ElementSetting *get_setting(GraphElement& element, const string& name) const
  {
    auto s = get_setting(name);
    if (!s)
      return nullptr;
    return &s->get(element);
  }
  ElementInput *get_input(GraphElement& element, const string& name) const
  {
    auto i = get_input(name);
    if (!i)
      return nullptr;
    return &i->get(element);
  }
  ElementOutput *get_output(GraphElement& element, const string& name) const
  {
    auto o = get_output(name);
    if (!o)
      return nullptr;
    return &o->get(element);
  }
  virtual const SettingMember *get_setting(const string& name) const = 0;
  virtual const InputMember *get_input(const string& name) const = 0;
  virtual const OutputMember *get_output(const string& name) const = 0;

  virtual bool has_settings() const = 0;
  virtual void for_each_setting(const function<void(const string&,
                                      const SettingMember&)>& func) const = 0;

  virtual bool has_inputs() const = 0;
  virtual void for_each_input(const function<void(const string&,
                                      const InputMember&)>& func) const = 0;

  virtual bool has_outputs() const = 0;
  virtual void for_each_output(const function<void(const string&,
                                         const OutputMember&)>& func) const = 0;

  virtual string get_input_id(GraphElement& element,
                              ElementInput& input) const = 0;
  virtual string get_output_id(GraphElement& element,
                               ElementOutput& output) const = 0;

  virtual ~Module() {}
};

//--------------------------------------------------------------------------
// Value type templates

// Generic (non-)definition
template<typename T> inline string get_module_type();
template<typename T>
inline void set_from_json(T& value, const JSON::Value& json);
template<typename T>
inline JSON::Value get_as_json(const T& value);

// Specialisation for <double>
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

// Specialisation for <string>
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

// Specialisation for <int>
template<>
inline string get_module_type<int>() { return "integer"; }

template<>
inline void set_from_json(int& value, const JSON::Value& json)
{
  if (json.type == JSON::Value::NUMBER)
    value = json.f;
  else
    value = json.n;
}

template<>
inline JSON::Value get_as_json(const int& value)
{
  return {value};
}

// Specialisation for <bool>
template<>
inline string get_module_type<bool>() { return "boolean"; }

template<>
inline void set_from_json(bool& value, const JSON::Value& json)
{
  value = (json.type == JSON::Value::TRUE_);
}

template<>
inline JSON::Value get_as_json(const bool& value)
{
  return {value ? JSON::Value::TRUE_ : JSON::Value::FALSE_};
}

class SimpleModule: public Module
{
protected:
  string id;
  string name;
  string section;

  // settings
  class Setting: public SettingMember
  {
  private:
    template<typename T>
    class TypedMember: public SettingMember
    {
    private:
      Dataflow::Setting<T> GraphElement::* member_pointer = nullptr;

    public:
      template<typename C>
      TypedMember(Dataflow::Setting<T> C::* _member_pointer):
        member_pointer{static_cast<Dataflow::Setting<T> GraphElement::*>(
                       _member_pointer)}
      {}

      string get_type() const override
      {
        return get_module_type<T>();
      }

      const ElementSetting& get(const GraphElement& b) const override
      {
        return b.*member_pointer;
      }

      ElementSetting& get(GraphElement& b) const override
      {
        return b.*member_pointer;
      }

      JSON::Value get_json(const GraphElement& b) const override
      {
        return get_as_json((b.*member_pointer).get());
      }

      void set_json(GraphElement& b, const JSON::Value& json) const override
      {
        auto v = T{};
        set_from_json(v, json);
        (b.*member_pointer).set(v);
      }

      void accept(ReadVisitor& visitor,
                  const Path& path, unsigned path_index,
                  const GraphElement& element) const override
      {
        if (path.reached(path_index))
          visitor.visit(element, *this, path, path_index);
      }

      void accept(WriteVisitor& visitor,
                  const Path& path, unsigned path_index,
                  GraphElement& element) const override
      {
        if (path.reached(path_index))
          visitor.visit(element, *this, path, path_index);
      }
    };
    shared_ptr<SettingMember> typed_member;

  public:
    template<typename T, typename C>
    Setting(Dataflow::Setting<T> C::* i):
      typed_member{new TypedMember<T>{i}}
    {}

    string get_type() const override
    {
      return typed_member->get_type();
    }

    const ElementSetting& get(const GraphElement& b) const override
    {
      return typed_member->get(b);
    }

    ElementSetting& get(GraphElement& b) const override
    {
      return typed_member->get(b);
    }

    JSON::Value get_json(const GraphElement& b) const override
    {
      return typed_member->get_json(b);
    }

    void set_json(GraphElement& b, const JSON::Value& json) const override
    {
      typed_member->set_json(b, json);
    }

    void accept(ReadVisitor& visitor,
                const Path& path, unsigned path_index,
                const GraphElement& element) const override
    {
      typed_member->accept(visitor, path, path_index, element);
    }

    void accept(WriteVisitor& visitor,
                const Path& path, unsigned path_index,
                GraphElement& element) const override
    {
      typed_member->accept(visitor, path, path_index, element);
    }
  };
  map<string, Setting> settings;

  class Input: public InputMember
  {
  private:
    template<typename T>
    class TypedMember: public InputMember
    {
    private:
      Dataflow::Input<T> GraphElement::* member_pointer = nullptr;

    public:
      template<typename C>
      TypedMember(Dataflow::Input<T> C::* _member_pointer):
        member_pointer{static_cast<Dataflow::Input<T> GraphElement::*>(
                      _member_pointer)}
      {}

      string get_type() const override
      {
        return get_module_type<T>();
      }

      const ElementInput& get(const GraphElement& b) const override
      {
        return b.*member_pointer;
      }

      ElementInput& get(GraphElement& b) const override
      {
        return b.*member_pointer;
      }

      JSON::Value get_json(const GraphElement& b) const override
      {
        return get_as_json((b.*member_pointer).get());
      }

      void set_json(GraphElement& b, const JSON::Value& json) const override
      {
        auto v = T{};
        set_from_json(v, json);
        (b.*member_pointer).set(v);
      }

      void accept(ReadVisitor& visitor,
                  const Path& path, unsigned path_index,
                  const GraphElement& element) const override
      {
        if (path.reached(path_index))
          visitor.visit(element, *this, path, path_index);
      }

      void accept(WriteVisitor& visitor,
                  const Path& path, unsigned path_index,
                  GraphElement& element) const override
      {
        if (path.reached(path_index))
          visitor.visit(element, *this, path, path_index);
      }

      double get_sample_rate(const GraphElement& b) const override
      {
        return (b.*member_pointer).get_sample_rate();
      }
    };
    shared_ptr<InputMember> typed_member;

  public:
    template<typename T, typename C>
    Input(Dataflow::Input<T> C::* i):
      typed_member{new TypedMember<T>{i}}
    {}

    string get_type() const override
    {
      return typed_member->get_type();
    }

    const ElementInput& get(const GraphElement& b) const override
    {
      return typed_member->get(b);
    }

    ElementInput& get(GraphElement& b) const override
    {
      return typed_member->get(b);
    }

    JSON::Value get_json(const GraphElement& b) const override
    {
      return typed_member->get_json(b);
    }

    void set_json(GraphElement& b, const JSON::Value& json) const override
    {
      typed_member->set_json(b, json);
    }

    void accept(ReadVisitor& visitor,
                const Path& path, unsigned path_index,
                const GraphElement& element) const override
    {
      typed_member->accept(visitor, path, path_index, element);
    }

    void accept(WriteVisitor& visitor,
                const Path& path, unsigned path_index,
                GraphElement& element) const override
    {
      typed_member->accept(visitor, path, path_index, element);
    }

    double get_sample_rate(const GraphElement& b) const override
    {
      return typed_member->get_sample_rate(b);
    }
  };
  map<string, Input> inputs;

  class Output: public OutputMember
  {
  private:
    template<typename T>
    class TypedMember: public OutputMember
    {
    private:
      Dataflow::Output<T> GraphElement::* member_pointer = nullptr;

    public:
      template<typename C>
      TypedMember(Dataflow::Output<T> C::* _member_pointer):
        member_pointer{static_cast<Dataflow::Output<T> GraphElement::*>(
                      _member_pointer)}
      {}

      string get_type() const override
      {
        return get_module_type<T>();
      }

      const ElementOutput& get(const GraphElement& b) const override
      {
        return b.*member_pointer;
      }

      ElementOutput& get(GraphElement& b) const override
      {
        return b.*member_pointer;
      }

      void accept(ReadVisitor& visitor,
                  const Path& path, unsigned path_index,
                  const GraphElement& element) const override
      {
        if (path.reached(path_index))
          visitor.visit(element, *this, path, path_index);
      }

      void accept(WriteVisitor& visitor,
                  const Path& path, unsigned path_index,
                  GraphElement& element) const override
      {
        if (path.reached(path_index))
          visitor.visit(element, *this, path, path_index);
      }

      double get_sample_rate(const GraphElement& b) const override
      {
        return (b.*member_pointer).get_sample_rate();
      }
    };
    shared_ptr<OutputMember> typed_member;

  public:
    template<typename T, typename C>
    Output(Dataflow::Output<T> C::* o):
      typed_member{new TypedMember<T>{o}}
    {}

    string get_type() const override
    {
      return typed_member->get_type();
    }

    const ElementOutput& get(const GraphElement& b) const override
    {
      return typed_member->get(b);
    }

    ElementOutput& get(GraphElement& b) const override
    {
      return typed_member->get(b);
    }

    void accept(ReadVisitor& visitor,
                const Path& path, unsigned path_index,
                const GraphElement& element) const override
    {
      typed_member->accept(visitor, path, path_index, element);
    }

    void accept(WriteVisitor& visitor,
                const Path& path, unsigned path_index,
                GraphElement& element) const override
    {
      typed_member->accept(visitor, path, path_index, element);
    }

    double get_sample_rate(const GraphElement& b) const override
    {
      return typed_member->get_sample_rate(b);
    }
  };
  map<string, Output> outputs;

public:
  SimpleModule(const string& _id, const string& _name, const string& _section,
               const map<string, Setting>& _settings,
               const map<string, Input>& _inputs,
               const map<string, Output>& _outputs):
    id{_id}, name{_name}, section{_section}, settings{_settings},
    inputs{_inputs}, outputs{_outputs}
  {}

  string get_id() const override { return id; }
  string get_name() const override { return name; }
  string get_section() const override { return section; }

  const SettingMember *get_setting(const string& name) const override
  {
    auto sit = settings.find(name);
    if (sit == settings.end())
      return nullptr;
    return &sit->second;
  }

  const InputMember *get_input(const string& name) const override
  {
    auto iit = inputs.find(name);
    if (iit == inputs.end())
      return nullptr;
    return &iit->second;
  }

  const OutputMember *get_output(const string& name) const override
  {
    auto oit = outputs.find(name);
    if (oit == outputs.end())
      return nullptr;
    return &oit->second;
  }

  bool has_settings() const override { return !settings.empty(); }
  void for_each_setting(const function<void(const string&,
                                  const SettingMember&)>& func) const override
  {
    for (const auto& sit: settings)
      func(sit.first, sit.second);
  }

  bool has_inputs() const override { return !inputs.empty(); }
  void for_each_input(const function<void(const string&,
                                    const InputMember&)>& func) const override
  {
    for (const auto& iit: inputs)
      func(iit.first, iit.second);
  }

  bool has_outputs() const override { return !outputs.empty(); }
  void for_each_output(const function<void(const string&,
                                   const OutputMember&)>& func) const override
  {
    for (const auto& oit: outputs)
      func(oit.first, oit.second);
  }

  string get_input_id(GraphElement& element,
                      ElementInput& input) const override
  {
    for (const auto& i: inputs)
      if (&i.second.get(element) == &input)
        return i.first;
    return "[invalid]";
  }

  string get_output_id(GraphElement& element,
                       ElementOutput& output) const override
  {
    for (const auto& o: outputs)
      if (&o.second.get(element) == &output)
        return o.first;
    return "[invalid]";
  }
};

//==========================================================================
// Dyanmic module information
class DynamicModule: public SimpleModule
{
public:
  using SimpleModule::SimpleModule;

  bool is_dynamic() const override { return true; }

  auto num_inputs() const
  {
    return inputs.size();
  }

  void clear_inputs()
  {
    inputs.clear();
  }

  template<typename T, typename C>
  void add_input(const string& name, Dataflow::Input<T> C::* i)
  {
    inputs.emplace(name, i);
  }

  void erase_input(const string& name)
  {
    inputs.erase(name);
  }

  auto num_outputs() const
  {
    return outputs.size();
  }

  void clear_outputs()
  {
    outputs.clear();
  }

  template<typename T, typename C>
  void add_output(const string& name, Dataflow::Output<T> C::* o)
  {
    outputs.emplace(name, o);
  }

  void erase_output(const string& name)
  {
    outputs.erase(name);
  }
};

//==========================================================================
// Graph element - just has an ID
class GraphElement
{
private:
  string id;

public:
  string get_id() const { return id; }

  virtual void set_id(const string& _id) { id = _id; }

  virtual const Module& get_module() const = 0;

  // Setup after automatic configuration
  virtual void setup() {}

  // Connect an element
  virtual bool connect(const string& out_name,
                       GraphElement& b, const string &in_name) = 0;

  // Notify that connection has been made to input
  virtual void notify_connection(const string& in_name,
                                 GraphElement& a, const string& out_name) = 0;

  // Handle sample rate being changed
  virtual void update_sample_rate() = 0;

  // Set a Setting/Input
  template<typename T>
  GraphElement& set(const string& setting, const T& value)
  {
    auto& module = get_module();
    auto s = dynamic_cast<Setting<T> *>(module.get_setting(*this, setting));
    if (s)
    {
      s->set(value);
    }
    else
    {
      auto i = dynamic_cast<Input<T> *>(module.get_input(*this, setting));
      if (i)
        i->set(value);
    }
    return *this;
  }

  // Update after setting a setting
  virtual void update() {}

  // Notify of parent graph being enabled - register for keys etc.
  virtual void enable() {}

  // Notify of parent graph being disabled - de-register for keys etc.
  virtual void disable() {}

  // Prepare for a tick
  virtual void reset() = 0;

  // Collect list of all elements
  virtual void collect_elements(list<Element *>& elements) = 0;

  // Accept visitors
  virtual void accept(ReadVisitor& visitor,
                      const Path& path, unsigned path_index) const = 0;
  virtual void accept(WriteVisitor& visitor,
                      const Path& path, unsigned path_index) = 0;

  // Clone element
  virtual GraphElement *clone() const = 0;

  // Clean shutdown
  virtual void shutdown() {}

  // Virtual destructor
  virtual ~GraphElement() {}
};

//==========================================================================
// Graph element - just has an ID and a parent graph
class Element: public GraphElement
{
private:
  std::set<ElementInput *> inputs;

  template<typename... Ss, size_t... Sc, typename... Is, size_t... Ic,
           typename... Os, size_t... Oc, typename F>
  void sample_iterate_impl(unsigned int count,
                           const tuple<Ss...>& ss, index_sequence<Sc...>,
                           const tuple<Is...>& is, index_sequence<Ic...>,
                           const tuple<Os...>& os, index_sequence<Oc...>,
                           const F& f)
  {
    auto settings = make_tuple(get<Sc>(ss).get()...);
    (void)settings;
    auto inputs = make_tuple(get<Ic>(is).get_buffer()...);
    (void)inputs;
    auto outputs = make_tuple(get<Oc>(os).get_buffer()...);
    (void)outputs;
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

  // Get a default constructed clone
  virtual Element *create_clone() const = 0;

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
  // Connect an element
  bool connect(const string& out_name,
               GraphElement& b, const string &in_name) override;

  // Notify that connection has been made to input
  void notify_connection(const string& in_name,
                         GraphElement& a, const string& out_name) override;

  // Handle sample rate change
  void update_sample_rate() override;

  // Is ready to process tick
  bool ready() const;

  // Tick
  virtual void tick(const TickData& /*tick data*/) {}

  // Prepare for a tick
  void reset() override;

  // Collect list of all elements
  void collect_elements(list<Element *>& elements) override
  {
    elements.push_back(this);
  }

  // Clone element
  Element *clone() const override;

  // Accept visitors
  void accept(ReadVisitor& visitor,
              const Path& path, unsigned path_index) const override;
  void accept(WriteVisitor& visitor,
              const Path& path, unsigned path_index) override;
};

//==========================================================================
// Element that has static module information
class SimpleElement: public Element
{
protected:
  const SimpleModule& module;

public:
  //------------------------------------------------------------------------
  // Constructor
  SimpleElement(const SimpleModule& _module):
    module{_module}
  {}

  const Module& get_module() const override
  {
    return module;
  }
};

//==========================================================================
// Element whose module information can change depending on settings
class DynamicElement: public Element
{
protected:
  DynamicModule module;

public:
  //------------------------------------------------------------------------
  // Constructor
  DynamicElement(const DynamicModule& _module):
    module{_module}
  {}

  const Module& get_module() const override
  {
    return module;
  }
};

class Generator;  // forward

//==========================================================================
// Graph module information
class GraphModule: public Module
{
private:
  class GraphInputMember: public InputMember
  {
  private:
    GraphElement& pin;
    const InputMember& module;
  public:
    GraphInputMember(GraphElement& _pin):
      pin{_pin}, module{[&_pin]() -> const InputMember&
      {
        auto m = _pin.get_module().get_input("input");
        if (!m)
          throw(runtime_error{"Could not find pin input"});
        return *m;
      }()}
    {}
    string get_type() const override
    {
      return module.get_type();
    }
    const ElementInput& get(const GraphElement&) const override
    {
      return module.get(pin);
    }
    ElementInput& get(GraphElement&) const override
    {
      return module.get(pin);
    }
    JSON::Value get_json(const GraphElement&) const override
    {
      return module.get_json(pin);
    }
    void set_json(GraphElement&, const JSON::Value& json) const override
    {
      return module.set_json(pin, json);
    }

    void accept(ReadVisitor& visitor,
                const Path& path, unsigned path_index,
                const GraphElement& element) const override
    {
      if (path.reached(path_index))
        visitor.visit(element, *this, path, path_index);
    }

    void accept(WriteVisitor& visitor,
                const Path& path, unsigned path_index,
                GraphElement& element) const override
    {
      if (path.reached(path_index))
        visitor.visit(element, *this, path, path_index);
    }

    double get_sample_rate(const GraphElement&) const override
    {
      return module.get_sample_rate(pin);
    }
  };
  class GraphOutputMember: public OutputMember
  {
  private:
    GraphElement& pin;
    const OutputMember& module;
  public:
    GraphOutputMember(GraphElement& _pin):
      pin{_pin}, module{[&_pin]() -> const OutputMember&
      {
        auto m = _pin.get_module().get_output("output");
        if (!m)
          throw(runtime_error{"Could not find pin output"});
        return *m;
      }()}
    {}
    string get_type() const override
    {
      return module.get_type();
    }
    const ElementOutput& get(const GraphElement&) const override
    {
      return module.get(pin);
    }
    ElementOutput& get(GraphElement&) const override
    {
      return module.get(pin);
    }

    void accept(ReadVisitor& visitor,
                const Path& path, unsigned path_index,
                const GraphElement& element) const override
    {
      if (path.reached(path_index))
        visitor.visit(element, *this, path, path_index);
    }

    void accept(WriteVisitor& visitor,
                const Path& path, unsigned path_index,
                GraphElement& element) const override
    {
      if (path.reached(path_index))
        visitor.visit(element, *this, path, path_index);
    }

    double get_sample_rate(const GraphElement&) const override
    {
      return module.get_sample_rate(pin);
    }
  };
  map<string, GraphInputMember> inputs;
  map<string, GraphOutputMember> outputs;
  friend class Graph;
  friend class Clone;

  string id;
  string name;
  string section;

public:
  GraphModule():
    id{"graph"}, name{"Graph"}, section{"core"}
  {}

  string get_id() const override { return id; }
  string get_name() const override { return name; }
  string get_section() const override { return section; }
  bool is_dynamic() const override { return true; }

  const SettingMember *get_setting( const string&) const override
  {
    return nullptr;
  }

  const InputMember *get_input(const string& name) const override
  {
    auto iit = inputs.find(name);
    if (iit == inputs.end())
      return nullptr;
    return &iit->second;
  }

  const OutputMember *get_output(const string& name) const override
  {
    auto oit = outputs.find(name);
    if (oit == outputs.end())
      return nullptr;
    return &oit->second;
  }

  bool has_settings() const override { return false; }
  void for_each_setting(const function<void(const string&,
                                  const SettingMember&)>&) const override
  {
  }

  bool has_inputs() const override { return !inputs.empty(); }
  void for_each_input(const function<void(const string&,
                                    const InputMember&)>& func) const override
  {
    for (const auto& iit: inputs)
      func(iit.first, iit.second);
  }

  bool has_outputs() const override { return !outputs.empty(); }
  void for_each_output(const function<void(const string&,
                                   const OutputMember&)>& func) const override
  {
    for (const auto& oit: outputs)
      func(oit.first, oit.second);
  }

  string get_input_id(GraphElement& element,
                      ElementInput& input) const override
  {
    for (const auto& i: inputs)
      if (&i.second.get(element) == &input)
        return i.first;
    return "[invalid]";
  }

  string get_output_id(GraphElement& element,
                       ElementOutput& output) const override
  {
    for (const auto& o: outputs)
      if (&o.second.get(element) == &output)
        return o.first;
    return "[invalid]";
  }
};

//==========================================================================
// Dataflow graph structure
class Graph: public GraphElement
{
private:
  mutable MT::RWMutex mutex;
  map<string, shared_ptr<GraphElement>> elements;   // By ID
  double sample_rate = 0;
  GraphModule module;

  struct PinInfo
  {
    string element;
    string connection;
    PinInfo(const string& _element, const string& _connection):
      element{_element}, connection{_connection}
    {}
  };
  map<string, PinInfo> input_pins;
  map<string, PinInfo> output_pins;

public:
  //------------------------------------------------------------------------
  // Constructors
  Graph(const GraphModule& _module = {}):
    module{_module}
  {}

  const Module& get_module() const override
  {
    return module;
  }

  // Connect an element
  bool connect(const string& out_name,
               GraphElement& b, const string &in_name) override;

  // Notify that connection has been made to input
  void notify_connection(const string& in_name,
                         GraphElement& a, const string& out_name) override;

  // Handle sample rate change
  void update_sample_rate() override {}

  //------------------------------------------------------------------------
  // Get all elements (for inspection)
  const map<string, shared_ptr<GraphElement> >& get_elements() const
  { return elements; }

  //------------------------------------------------------------------------
  // Add an element to the graph
  void add(GraphElement *el);

  //------------------------------------------------------------------------
  // Add input pin
  void add_input_pin(const string& id,
                     const string& element, const string& input);

  //------------------------------------------------------------------------
  // Remove an input pin
  void remove_input_pin(const string& id);

  //------------------------------------------------------------------------
  // Add output pin
  void add_output_pin(const string& id,
                      const string& element, const string& output);

  //------------------------------------------------------------------------
  // Remove an output pin
  void remove_output_pin(const string& id);

  //------------------------------------------------------------------------
  // Final setup for elements and calculate topology
  void setup() override;

  //------------------------------------------------------------------------
  // Set sample rate
  void set_sample_rate(double sr) { sample_rate = sr; }

  //------------------------------------------------------------------------
  // Get a particular element by ID
  GraphElement *get_element(const string& id);

  //------------------------------------------------------------------------
  // Remove an element
  void remove(const string& id);

  //------------------------------------------------------------------------
  // Clear all elements
  void clear_elements()
  {
    elements.clear();
  }

  //------------------------------------------------------------------------
  // Prepare for a tick
  void reset() override
  {
    for (auto it: elements)
      it.second->reset();
  }

  // Collect list of all elements
  void collect_elements(list<Element *>& els) override
  {
    for (auto it: elements)
      it.second->collect_elements(els);
  }

  // Clone element
  Graph *clone() const override;

  //------------------------------------------------------------------------
  // Accept visitors
  void accept(ReadVisitor& visitor,
              const Path& path, unsigned path_index) const override;
  void accept(WriteVisitor& visitor,
              const Path& path, unsigned path_index) override;

  //------------------------------------------------------------------------
  // Shutdown all elements
  void shutdown() override;
};

class CloneInfo;

//==========================================================================
// Clone module
class CloneModule: public SimpleModule
{
public:
  using SimpleModule::SimpleModule;

  bool is_dynamic() const override { return true; }
};

//==========================================================================
// Dataflow clone structure
class Clone: public GraphElement
{
private:
  struct CloneGraph
  {
    shared_ptr<Graph> graph;
    std::set<CloneInfo *> infos;
    CloneGraph():
      graph{make_shared<Graph>()}
    {}
    CloneGraph(Graph *_graph):
      graph{_graph}
    {}
  };
  vector<CloneGraph> clones;
  const CloneModule& module;

  // Update CloneInfo objects
  void update_clone_infos();

public:
  //------------------------------------------------------------------------
  // Constructors
  Clone(const CloneModule& _module):
    module{_module}
  {
    clones.emplace_back();
  }

  Setting<double> number;

  void set_id(const string& _id) override;

  const Module& get_module() const override;

  // Connect an element
  bool connect(const string& out_name,
               GraphElement& b, const string &in_name) override;

  // Notify that connection has been made to input
  void notify_connection(const string& in_name,
                         GraphElement& a, const string& out_name) override;

  // Handle sample rate change
  void update_sample_rate() override {}

  //------------------------------------------------------------------------
  // Register a clone info
  void register_info(const Graph& graph, CloneInfo *info);

  //------------------------------------------------------------------------
  // Final setup for elements and calculate topology
  void setup() override;

  //------------------------------------------------------------------------
  // Prepare for a tick
  void reset() override;

  // Collect list of all elements
  void collect_elements(list<Element *>& els) override;

  // Clone element
  Clone *clone() const override;

  //------------------------------------------------------------------------
  // Accept visitors
  void accept(ReadVisitor& visitor,
              const Path& path, unsigned path_index) const override;
  void accept(WriteVisitor& visitor,
              const Path& path, unsigned path_index) override;

  //------------------------------------------------------------------------
  // Shutdown all elements
  void shutdown() override;
};

//==========================================================================
// Clone Module
const Dataflow::CloneModule clone_module
{
  "clone",
  "Clone",
  "core",
  {
    { "number", &Clone::number }
  },
  {},
  {}
};

//==========================================================================
// Clone Info
class CloneInfo: public SimpleElement
{
private:
  friend class Clone;

  double clone_number = 0;
  double clone_total = 0;

  void tick(const TickData& td) override;

  CloneInfo *create_clone() const override
  {
    return new CloneInfo{module};
  }
public:
  using SimpleElement::SimpleElement;

  // Info Outputs
  Output<double> number;
  Output<double> total;
  Output<double> fraction;
};

const Dataflow::SimpleModule clone_info_module
{
  "clone-info",
  "Clone Information",
  "core",
  {},
  {},
  {
    { "number", &CloneInfo::number },
    { "total", &CloneInfo::total },
    { "fraction", &CloneInfo::fraction },
  }
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
// Pin class template
template<typename T>
class Pin: public SimpleElement
{
private:
  void tick(const TickData&) override
  {
    output.get_buffer().data = input.get_buffer();
  }
public:
  using SimpleElement::SimpleElement;
  Input<T> input;
  Output<T> output;
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
    virtual GraphElement *create() const = 0;
    virtual const Module *get_module() const = 0;
    virtual ~Factory() {}
  };

  // Template for factories creating with { new Type }
  template<class E, typename M> class NewFactory: public Factory
  {
  private:
    const M& module;
  public:
    NewFactory(const M& _module):
      module{_module}
    {}
    GraphElement *create() const override
    { return new E{module}; }
    const Module *get_module() const override
    { return &module; }
  };

  struct Section
  {
    map<string, const Factory *> modules;
  };

  map<string, Section> sections;

  //------------------------------------------------------------------------
  // Constructor
  Registry() {}

  //------------------------------------------------------------------------
  // Register a module with its factory
  void add(const string& section, const string& id, const Factory& f)
  { sections[section].modules[id] = &f; }

  //------------------------------------------------------------------------
  // Create an object by module and config
  // Returns the object, or 0 if no factories available or create fails
  GraphElement *create(const string& section, const string& id) const
  {
    const auto sp = sections.find(section);
    if (sp == sections.end()) return 0;

    const auto mp = sp->second.modules.find(id);
    if (mp == sp->second.modules.end()) return 0;

    const auto& factory = mp->second;
    return factory->create();
  }
};

//==========================================================================
// Engine class - wrapper containing Graph tree and Element registry
class Engine
{
  // Graph structure
  mutable MT::RWMutex graph_mutex;
  unique_ptr<Dataflow::Graph> graph;
  list<Element *> tick_elements;
  Time::Duration tick_interval = default_tick_interval;
  double sample_rate = default_sample_rate;
  Time::Stamp start_time;
  uint64_t tick_number{0};
  list<string> default_sections;  // Note: ordered

 public:
  Registry element_registry;

  //------------------------------------------------------------------------
  // Constructor
  Engine(): graph(new Graph{})
  {
    graph->set_id("root");
  }

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
  Dataflow::Graph& get_graph() const { return *graph; }

  //------------------------------------------------------------------------
  // Create an element with the given type - may be section:id or just id,
  // which is looked up in default namespaces
  GraphElement *create(const string& type, const string& id) const;

  //------------------------------------------------------------------------
  // Update element list
  void update_elements();

  //------------------------------------------------------------------------
  // Tick the graph
  void tick(Time::Stamp t);

  //------------------------------------------------------------------------
  // Accept visitors
  void accept(ReadVisitor& visitor,
              const Path& path, unsigned path_index) const;
  void accept(WriteVisitor& visitor,
              const Path& path, unsigned path_index);

  //------------------------------------------------------------------------
  // Reset
  void reset()
  {
    start_time = {};
    tick_number = {};
  }

  //------------------------------------------------------------------------
  // Shut down the graph
  void shutdown();
};

//==========================================================================
}} //namespaces
#endif // !__VG_DATAFLOW_H
