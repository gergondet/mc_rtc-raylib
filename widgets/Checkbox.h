#pragma once

#include "Widget.h"

struct Checkbox : public Widget
{
  inline Checkbox(const ElementId & id) : Widget(id) {}

  ~Checkbox() override = default;

  inline void data(bool data)
  {
    data_ = data;
  }

  inline void draw2D() override
  {
    if(ImGui::Checkbox(label(id.name).c_str(), &data_))
    {
      clicked_ = true;
    }
  }

  inline void update(Client & client, SceneState & state) override
  {
    if(clicked_)
    {
      client.send_request(id);
      clicked_ = false;
    }
  }

private:
  bool data_ = false;
  bool clicked_ = false;
};
