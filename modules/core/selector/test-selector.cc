//==========================================================================
// ViGraph dataflow module: core/selector/test-selector.cc
//
// Tests for <selector> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"
#include "vg-waveform.h"
#include <cmath>

class SelectorTest: public GraphTester
{
public:
  SelectorTest()
  {
    loader.load("./vg-module-core-selector.so");
  }
};

const auto nsamples = 10;

TEST_F(SelectorTest, TestSelectorNotTriggered)
{
  auto& selector = add("core/selector").set("inputs", Integer{3});
  setup(selector);

  auto output = vector<Number>{};
  auto& sink = add_sink(output, nsamples);
  selector.connect("output", sink, "input");

  run();

  ASSERT_EQ(nsamples, output.size());
  for(auto i=0u; i<nsamples; i++)
    EXPECT_EQ(0, output[i]);
}

TEST_F(SelectorTest, TestSelectorTriggeredOnce)
{
  auto& selector = add("core/selector").set("inputs", Integer{3});
  setup(selector);

  auto i1_data = vector<Trigger>(nsamples);
  i1_data[0] = 1;
  auto& i1s = add_source(i1_data);
  i1s.connect("output", selector, "select1");

  auto output = vector<Number>{};
  auto& sink = add_sink(output, nsamples);
  selector.connect("output", sink, "input");

  run();

  ASSERT_EQ(nsamples, output.size());
  for(auto i=0u; i<nsamples; i++)
    EXPECT_EQ(1, output[i]);
}

TEST_F(SelectorTest, TestSelectorTriggeredTwiceSeparately)
{
  auto& selector = add("core/selector").set("inputs", Integer{3});
  setup(selector);

  auto i1_data = vector<Trigger>(nsamples);
  i1_data[0] = 1;
  auto& i1s = add_source(i1_data);
  i1s.connect("output", selector, "select1");

  auto i2_data = vector<Trigger>(nsamples);
  i2_data[nsamples/2] = 1;
  auto& i2s = add_source(i2_data);
  i2s.connect("output", selector, "select2");

  auto output = vector<Number>{};
  auto& sink = add_sink(output, nsamples);
  selector.connect("output", sink, "input");

  run();

  ASSERT_EQ(nsamples, output.size());
  for(auto i=0u; i<nsamples; i++)
    EXPECT_EQ(i<nsamples/2 ? 1 : 2, output[i]);
}

TEST_F(SelectorTest, TestSelectorTriggeredThriceOverlaid)
{
  auto& selector = add("core/selector").set("inputs", Integer{3});
  setup(selector);

  auto i1_data = vector<Trigger>(nsamples);
  i1_data[0] = 1;
  auto& i1s = add_source(i1_data);
  i1s.connect("output", selector, "select1");

  auto i2_data = vector<Trigger>(nsamples);
  i2_data[nsamples/2] = 1;
  auto& i2s = add_source(i2_data);
  i2s.connect("output", selector, "select2");

  auto i3_data = vector<Trigger>(nsamples);
  i3_data[nsamples/2] = 1;
  auto& i3s = add_source(i3_data);
  i3s.connect("output", selector, "select3");

  auto output = vector<Number>{};
  auto& sink = add_sink(output, nsamples);
  selector.connect("output", sink, "input");

  run();

  ASSERT_EQ(nsamples, output.size());
  for(auto i=0u; i<nsamples; i++)
    EXPECT_EQ(i<nsamples/2 ? 1 : 3, output[i]);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
