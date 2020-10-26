#pragma once

#include "Widget.h"

struct Point3D : public Widget
{
  Point3D(Client & client, const ElementId & id, const ElementId & requestId);

  ~Point3D() override = default;

  void data(bool ro, const Eigen::Vector3d & pos, const mc_rtc::gui::PointConfig & config);

  void update(SceneState & state) override;

  void draw3D(Camera camera) override;

private:
  ElementId requestId_;
  Vector3 pos_;
  mc_rtc::gui::PointConfig config_;
  std::unique_ptr<InteractiveMarker> marker_;
};
