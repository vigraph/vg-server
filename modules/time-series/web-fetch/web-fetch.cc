//==========================================================================
// ViGraph dataflow module: time-series/web-fetch/web-fetch.cc
//
// Web fetch data source
//
// Copyright (c) 2018-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../time-series-module.h"
#include "ot-web.h"

namespace {

const auto user_agent("ViGraph Web fetcher/2.0");
const auto fetch_timeout{5};

//==========================================================================
// SVG source
class WebFetch: public SimpleElement
{
private:
  DataSet data;

  // Source/Element virtuals
  void setup() override;
  void tick(const TickData& td) override;

  // Clone
  WebFetch *create_clone() const override
  {
    return new WebFetch{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Settings
  Setting<string> url;
  Setting<string> name;

  // Output
  Output<DataSet> output;
};

//--------------------------------------------------------------------------
// Setup
void WebFetch::setup()
{
  Log::Streams log;
  Web::URL wurl(url);
  Web::HTTPClient client(wurl, 0, user_agent, fetch_timeout, fetch_timeout);
  string body;
  log.detail << "Fetching data from " << wurl << endl;
  auto res = client.get(wurl, body);
  if (res != 200)
  {
    log.error << "Fetch from " << wurl << " failed: " << res << " " << body
              << endl;
    return;
  }

  data.source = url;
  data.name = name;

  // !WoodForTrees format only at the moment
  // Line comments with '#'
  // Time and value data pair on each line

  // Read the body
  istringstream iss(body);
  Lex::Analyser lex(iss);
  lex.add_line_comment_symbol("#");

  auto from = -1.0;
  auto t{0.0};
  for(;;)
  {
    Lex::Token token = lex.read_token();
    if (token.type == Lex::Token::END) break;
    if (token.type == Lex::Token::NUMBER)
    {
      t = Text::stof(token.value);
      if (from < 0) from = t;

      token = lex.read_token();
      if (token.type == Lex::Token::END) break;
      if (token.type == Lex::Token::NUMBER)
      {
        double v = Text::stof(token.value);
        data.add(t, v);
      }
    }
  }

  auto interval = (data.samples.size()>1)?(t-from)/(data.samples.size()-1):0;

  log.detail << "Read " << data.samples.size() << " values from " << from
             << " at interval " << interval << endl;
}

//--------------------------------------------------------------------------
// Generate a frame
void WebFetch::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(nsamples, {}, {}, tie(output),
                 [&](DataSet& output)
  {
    output = data;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "web-fetch",
  "Web fetch",
  "time-series",
  {
    { "url",      &WebFetch::url     },
    { "name",     &WebFetch::name    }
  },
  {},
  {
    { "output",   &WebFetch::output   }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(WebFetch, module)
