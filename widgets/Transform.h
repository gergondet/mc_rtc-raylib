#pragma once

#include "details/TransformBase.h"

struct TransformWidget : public TransformBase<ControlAxis::ALL>
{
  TransformWidget(Client & client, const ElementId & id, const ElementId & reqId) : TransformBase(client, id, reqId) {}

  void draw3D(Camera camera) override
  {
    TransformBase::draw3D(camera);
    DrawFrame(pos_);
  }

private:
  ElementId requestId_;
};
