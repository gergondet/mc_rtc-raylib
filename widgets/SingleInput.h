#pragma once

#include "Widget.h"

template<typename DataT>
struct SingleInput : public Widget
{
  inline SingleInput(const ElementId & id) : Widget(id) {}

  ~SingleInput() override = default;

  inline void data(const DataT & data)
  {
    if(!busy_)
    {
      data_ = data;
    }
  }

  virtual void setupBuffer() {}

  virtual DataT dataFromBuffer() = 0;

  template<typename ImGuiFn, typename... Args>
  void draw2D(ImGuiFn fn, Args &&... args)
  {
    ImGui::Text("%s", id.name.c_str());
    ImGui::SameLine();
    if(!busy_)
    {
      if(ImGui::Button("Edit"))
      {
        busy_ = true;
        setupBuffer();
      }
      fn("", std::forward<Args>(args)..., ImGuiInputTextFlags_ReadOnly);
    }
    else
    {
      if(ImGui::Button("Done")
         || fn(fmt::format("##{}{}Input", id.category, id.name).c_str(), std::forward<Args>(args)...,
               ImGuiInputTextFlags_EnterReturnsTrue))
      {
        busy_ = false;
        auto nData = dataFromBuffer();
        if(nData != data_)
        {
          changed_ = true;
          data_ = nData;
        }
      }
    }
  }

  inline void update(Client & client, SceneState & state) override
  {
    if(changed_)
    {
      changed_ = false;
      client.send_request(id, data_);
    }
  }

protected:
  bool busy_ = false;
  bool changed_ = false;
  DataT data_;
};
