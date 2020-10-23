#pragma once

#include "Widget.h"

struct ArrayInput : public Widget
{
  inline ArrayInput(const ElementId & id) : Widget(id) {}

  ~ArrayInput() override = default;

  inline bool data(const std::vector<std::string> & labels, const Eigen::VectorXd & data)
  {
    if(!busy_)
    {
      labels_ = labels;
      data_ = data;
    }
  }

  inline void draw2D() override
  {
    int flags = busy_ ? ImGuiInputTextFlags_EnterReturnsTrue : ImGuiInputTextFlags_ReadOnly;
    int columns = labels_.size() ? 3 : 2;
    double * source = busy_ ? buffer_.data() : data_.data();
    bool edit_done_ = false;
    ImGui::Columns(columns);
    ImGui::Text("%s", id.name.c_str());
    edit_done_ = ImGui::Button(label(busy_ ? "Done" : "Edit").c_str());
    ImGui::NextColumn();
    if(labels_.size())
    {
      for(const auto & l : labels_)
      {
        ImGui::Text("%s", l.c_str());
      }
      ImGui::NextColumn();
    }
    for(int i = 0; i < data_.size(); ++i)
    {
      edit_done_ = ImGui::InputDouble(label("", i).c_str(), &source[i], 0.0, 0.0, "%.6g", flags) || edit_done_;
    }
    if(edit_done_)
    {
      if(busy_)
      {
        changed_ = buffer_ != data_;
        busy_ = changed_;
      }
      else
      {
        buffer_ = data_;
        busy_ = true;
      }
    }
    ImGui::Columns(1);
  }

  inline void update(Client & client, SceneState & state) override
  {
    if(changed_)
    {
      data_ = buffer_;
      client.send_request(id, data_);
      changed_ = false;
      busy_ = false;
    }
  }

private:
  bool busy_ = false;
  bool changed_ = false;
  std::vector<std::string> labels_;
  Eigen::VectorXd data_;
  Eigen::VectorXd buffer_;
};
