#pragma once

#include "SingleInput.h"

struct IntegerInput : public SingleInput<int>
{
  inline IntegerInput(const ElementId & id) : SingleInput<int>(id) {}

  ~IntegerInput() override = default;

  void setupBuffer() override
  {
    buffer_ = data_;
  }

  int dataFromBuffer() override
  {
    return buffer_;
  }

  inline void draw2D() override
  {
    int * data = busy_ ? &buffer_ : &data_;
    SingleInput::draw2D(ImGui::InputInt, data, 0, 0);
  }

private:
  int buffer_;
};
