//==========================================================================
// ViGraph dataflow module: time-series/web-fetch/web-fetch.cc
//
// Web fetch data source
//
// Copyright (c) 2018-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"
#include "ot-web.h"

namespace {

const auto user_agent("ViGraph Web fetcher/2.0");
const auto fetch_timeout{5};

//==========================================================================
// SVG source
class WebFetch: public SimpleElement
{
private:
  vector<double> data;
  double _from;
  double _interval;

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

  // Output
  Output<double> output;
  Output<double> from;
  Output<double> interval;
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

  // !WoodForTrees format only at the moment
  // Line comments with '#'
  // Time and value data pair on each line

  // Read the body
  istringstream iss(body);
  Lex::Analyser lex(iss);
  lex.add_line_comment_symbol("#");

  _from = -1.0;
  auto t{0.0};
  for(;;)
  {
    Lex::Token token = lex.read_token();
    if (token.type == Lex::Token::END) break;
    if (token.type == Lex::Token::NUMBER)
    {
      t = Text::stof(token.value);
      if (_from < 0) _from = t;

      token = lex.read_token();
      if (token.type == Lex::Token::END) break;
      if (token.type == Lex::Token::NUMBER)
      {
        double v = Text::stof(token.value);
        data.push_back(v);
      }
    }
  }

  if (data.size()>1) _interval = (t-_from)/(data.size()-1);

  log.detail << "Read " << data.size() << " values from " << _from
             << " at interval " << _interval << endl;
}

//--------------------------------------------------------------------------
// Generate a frame
void WebFetch::tick(const TickData&)
{
  output.get_buffer().data = data;
  from.get_buffer().data.push_back(_from);
  interval.get_buffer().data.push_back(_interval);
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
  },
  {},
  {
    { "output",   &WebFetch::output   },
    { "from",     &WebFetch::from     },
    { "interval", &WebFetch::interval }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(WebFetch, module)
