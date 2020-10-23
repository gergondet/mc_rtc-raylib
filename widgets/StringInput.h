#pragma once

#include "Widget.h"

struct StringInput : public Widget
{
  inline StringInput(const ElementId & id) : Widget(id) {}

  ~StringInput() override = default;

  inline void data(const std::string & data)
  {
    if(!text_busy_)
    {
      text_ = data;
    }
  }

  inline void draw2D() override
  {
    ImGui::Text("%s", id.name.c_str());
    ImGui::SameLine();
    if(!text_busy_)
    {
      if(ImGui::Button("Edit"))
      {
        text_busy_ = true;
        buffer_.resize(std::max<size_t>(256, text_.size() + 1));
        std::memcpy(&buffer_[0], text_.c_str(), text_.size() + 1);
      }
      ImGui::InputText("", text_.data(), text_.size(), ImGuiInputTextFlags_ReadOnly);
    }
    else
    {
      if(ImGui::Button("Done")
         || ImGui::InputText(fmt::format("##{}{}InputText", id.category, id.name).c_str(), buffer_.data(),
                             buffer_.size(), ImGuiInputTextFlags_EnterReturnsTrue))
      {
        text_busy_ = false;
        std::string ntext(buffer_.data(), strnlen(buffer_.data(), buffer_.size()));
        if(ntext != text_)
        {
          text_ = ntext;
          text_changed_ = true;
        }
      }
    }
  }

  inline void update(Client & client, SceneState & state) override
  {
    if(text_changed_)
    {
      client.send_request(id, text_);
      text_changed_ = false;
    }
  }

private:
  bool text_busy_;
  bool text_changed_;
  std::string text_;
  std::vector<char> buffer_;
};
