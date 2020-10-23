#pragma once

#include "Widget.h"

struct Button : public Widget
{
  inline Button(const ElementId & id) : Widget(id) {}

  ~Button() override = default;

  inline void draw2D() override
  {
    if(ImGui::Button(label(id.name).c_str()))
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
  bool clicked_ = false;
};
