//==========================================================================
// ViGraph time-series modules: time-series-module.h
//
// Common definitions for time-series modules
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_TIME_SERIES_MODULE_H
#define __VIGRAPH_TIME_SERIES_MODULE_H

#include "../module.h"
#include "vg-dataflow.h"

namespace ViGraph { namespace Module { namespace TimeSeries {

//==========================================================================
// Single dataset
struct DataSet
{
  struct Sample
  {
    double at;
    double value;
    Sample(): at(0.0), value(0.0) {}
    Sample(double _at, double _value): at(_at), value(_value) {}
  };

  vector<Sample> samples;
  string name;             // Readable name for plotting
  string source;           // Source information - e.g. URL

  // !!! Start time and interval - how to represent from us to ma?

  DataSet() {}
  void add(double at, double value) { samples.push_back({ at, value }); }
};

//==========================================================================
// Multiple datasets
struct DataCollection
{
  vector<DataSet> datasets;

  // Add a dataset
  void add(const DataSet& ds) { datasets.push_back(ds); }

  // Combine with another one
  DataCollection& operator+=(const DataCollection& o)
  {
    datasets.insert(datasets.end(), o.datasets.begin(), o.datasets.end());
    return *this;
  }
};

typedef shared_ptr<DataCollection> DataCollectionPtr;

}}} //namespaces

using namespace ViGraph::Module::TimeSeries;

//==========================================================================
// JSON conversions
namespace ViGraph { namespace Dataflow {

template<> inline
string get_module_type<DataCollection>() { return "dataset"; }

template<> inline void set_from_json(DataCollection& datac,
                                     const JSON::Value& json)
{
  if (json.type == JSON::Value::ARRAY)
  {
    for (const auto& dsj: json.a)
    {
      DataSet dataset;
      if (dsj.type == JSON::Value::OBJECT)
      {
        for(const JSON::Value& js: dsj["samples"].a)
          dataset.add(js["at"].as_float(), js["value"].as_float());

        dataset.name = json["name"].as_str();
        dataset.source = json["source"].as_str();
      }
      datac.add(dataset);
    }
  }
}

template<> inline JSON::Value get_as_json(const DataCollection& datac)
{
  JSON::Value value{JSON::Value::ARRAY};
  for(const auto& dataset: datac.datasets)
  {
    auto& dsj = value.add(JSON::Value::OBJECT);
    JSON::Value& samples = dsj.put("samples", JSON::Value::ARRAY);
    for(const auto& s: dataset.samples)
    {
      JSON::Value &js = samples.add(JSON::Value::OBJECT);
      js.set("at", s.at).set("value", s.value);
    }

    dsj.set("name", dataset.name);
    dsj.set("source", dataset.source);
  }
  return value;
}

}} //namespaces

#endif // !__VIGRAPH_TIME_SERIES_MODULE_H
