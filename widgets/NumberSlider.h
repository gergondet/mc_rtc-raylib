#pragma once

#include "Widget.h"

struct NumberSlider : public Widget
{
  inline NumberSlider(const ElementId & id) : Widget(id) {}

  ~NumberSlider() override = default;

  inline void data(double data, double min, double max)
  {
    data_ = data;
    min_ = min;
    max_ = max;
  }

  inline void draw2D() override
  {
    if(ImGui::SliderFloat(id.name.c_str(), &data_, min_, max_))
    {
      changed_ = true;
    }
  }

  inline void update(Client & client, SceneState & state) override
  {
    if(changed_)
    {
      client.send_request(id, data_);
      changed_ = false;
    }
  }

private:
  bool changed_ = false;
  float data_;
  float min_;
  float max_;
};
