#pragma once

#include "ComboInput.h"

struct DataComboInput : public ComboInput
{
  using ComboInput::ComboInput;

  ~DataComboInput() override = default;

  inline void data(const std::vector<std::string> & values, const std::string & data)
  {
    data_ = data;
    mc_rtc::Configuration out = static_;
    if(values.size() > 1)
    {
      for(size_t i = 0; i < values.size() - 1; ++i)
      {
        if(!out.has(values[i]))
        {
          values_ = {};
          return;
        }
      }
    }
    values_ = out(values.back(), std::vector<std::string>{});
  }

  inline void update(Client & client, SceneState & state) override
  {
    static_ = client.data();
    ComboInput::update(client, state);
  }

private:
  mc_rtc::Configuration static_;
};
