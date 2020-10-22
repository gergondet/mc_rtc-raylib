#pragma once

#include "Widget.h"

struct Label : public Widget
{
  inline Label(const std::string & name) : Widget(name) {}

  ~Label() override = default;

  void data(const std::string & txt)
  {
    txt_ = txt;
  }

  void draw2D() override
  {
    ImGui::LabelText(txt_.c_str(), "%s", name.c_str());
  }
private:
  std::string txt_;
};
