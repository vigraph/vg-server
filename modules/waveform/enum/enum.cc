//==========================================================================
// Waveform::Type enumeration module
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../waveform-module.h"

namespace {

//==========================================================================
// None enum
class None: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  None *create_clone() const override
  {
    return new None{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Output<Waveform::Type> output;
};

//--------------------------------------------------------------------------
// Tick data
void None::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, {}, tie(output),
                 [&](Waveform::Type& o)
  {
    o = Waveform::Type::none;
  });
}

Dataflow::SimpleModule none_module
{
  "none",
  "None",
  "waveform",
  {},
  {},
  {
    { "output", &None::output }
  }
};

//==========================================================================
// Saw enum
class Saw: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Saw *create_clone() const override
  {
    return new Saw{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Output<Waveform::Type> output;
};

//--------------------------------------------------------------------------
// Tick data
void Saw::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, {}, tie(output),
                 [&](Waveform::Type& o)
  {
    o = Waveform::Type::saw;
  });
}

Dataflow::SimpleModule saw_module
{
  "saw",
  "Saw",
  "waveform",
  {},
  {},
  {
    { "output", &Saw::output }
  }
};

//==========================================================================
// Sin enum
class Sin: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Sin *create_clone() const override
  {
    return new Sin{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Output<Waveform::Type> output;
};

//--------------------------------------------------------------------------
// Tick data
void Sin::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, {}, tie(output),
                 [&](Waveform::Type& o)
  {
    o = Waveform::Type::sin;
  });
}

Dataflow::SimpleModule sin_module
{
  "sin",
  "Sin",
  "waveform",
  {},
  {},
  {
    { "output", &Sin::output }
  }
};

//==========================================================================
// Square enum
class Square: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Square *create_clone() const override
  {
    return new Square{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Output<Waveform::Type> output;
};

//--------------------------------------------------------------------------
// Tick data
void Square::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, {}, tie(output),
                 [&](Waveform::Type& o)
  {
    o = Waveform::Type::square;
  });
}

Dataflow::SimpleModule square_module
{
  "square",
  "Square",
  "waveform",
  {},
  {},
  {
    { "output", &Square::output }
  }
};

//==========================================================================
// Triangle enum
class Triangle: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Triangle *create_clone() const override
  {
    return new Triangle{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Output<Waveform::Type> output;
};

//--------------------------------------------------------------------------
// Tick data
void Triangle::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, {}, tie(output),
                 [&](Waveform::Type& o)
  {
    o = Waveform::Type::triangle;
  });
}

Dataflow::SimpleModule triangle_module
{
  "triangle",
  "Triangle",
  "waveform",
  {},
  {},
  {
    { "output", &Triangle::output }
  }
};

//==========================================================================
// Random enum
class Random: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Random *create_clone() const override
  {
    return new Random{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Output<Waveform::Type> output;
};

//--------------------------------------------------------------------------
// Tick data
void Random::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, {}, tie(output),
                 [&](Waveform::Type& o)
  {
    o = Waveform::Type::random;
  });
}

Dataflow::SimpleModule random_module
{
  "random",
  "Random",
  "waveform",
  {},
  {},
  {
    { "output", &Random::output }
  }
};

} // anon

Registry::NewFactory<None, decltype(none_module)> none_factory{none_module};
Registry::NewFactory<Saw, decltype(saw_module)> saw_factory{saw_module};
Registry::NewFactory<Sin, decltype(sin_module)> sin_factory{sin_module};
Registry::NewFactory<Square, decltype(square_module)>
    square_factory{square_module};
Registry::NewFactory<Triangle, decltype(triangle_module)>
    triangle_factory{triangle_module};
Registry::NewFactory<Random, decltype(random_module)>
    random_factory{random_module};
#if defined(PLATFORM_WEB)
extern "C" bool vg_init(Log::Channel&, Dataflow::Engine& engine)
{
  Log::Streams log;
#else
extern "C" bool vg_init(Log::Channel& logger, Dataflow::Engine& engine)
{
  Log::Streams log;
  Log::logger.connect(new Log::ReferencedChannel{logger});
#endif
  log.summary << "  Module: " << none_module.get_full_type() << endl;
  engine.element_registry.add(none_module.get_section(), none_module.get_id(),
                              none_factory);
  log.summary << "  Module: " << saw_module.get_full_type() << endl;
  engine.element_registry.add(saw_module.get_section(), saw_module.get_id(),
                              saw_factory);
  log.summary << "  Module: " << sin_module.get_full_type() << endl;
  engine.element_registry.add(sin_module.get_section(), sin_module.get_id(),
                              sin_factory);
  log.summary << "  Module: " << square_module.get_full_type() << endl;
  engine.element_registry.add(square_module.get_section(),
                              square_module.get_id(),
                              square_factory);
  log.summary << "  Module: " << triangle_module.get_full_type() << endl;
  engine.element_registry.add(triangle_module.get_section(),
                              triangle_module.get_id(),
                              triangle_factory);
  log.summary << "  Module: " << random_module.get_full_type() << endl;
  engine.element_registry.add(random_module.get_section(),
                              random_module.get_id(),
                              random_factory);
  return true;
}
