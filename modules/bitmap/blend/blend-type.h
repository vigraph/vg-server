//==========================================================================
// ViGraph bitmap graphics modules: blend-type.h
//
// Blend types
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_BITMAP_BLEND_TYPE_H
#define __VIGRAPH_BITMAP_BLEND_TYPE_H

//==========================================================================
// JSON conversions
namespace ViGraph { namespace Dataflow {

enum class BlendType
{
  none,
  horizontal,
  vertical,
  rectangular,
  radial
};

template<> inline
string get_module_type<BlendType>() { return "blend-type"; }

template<> inline void set_from_json(BlendType& type,
                                     const JSON::Value& json)
{
  const auto& m = json.as_str();

  if (m == "horizontal")
    type = BlendType::horizontal;
  else if (m == "vertical")
    type = BlendType::vertical;
  else if (m == "rectangular")
    type = BlendType::rectangular;
  else if (m == "radial")
    type = BlendType::radial;
  else
    type = BlendType::none;
}

template<> inline JSON::Value get_as_json(const BlendType& type)
{
  switch (type)
  {
    case BlendType::none:
      return "none";
    case BlendType::horizontal:
      return "horizontal";
    case BlendType::vertical:
      return "vertical";
    case BlendType::rectangular:
      return "rectangular";
    case BlendType::radial:
      return "radial";
  }
}

}} //namespaces

#endif // !__VIGRAPH_BITMAP_BLEND_TYPE_H
