#pragma once

#include "Widget.h"

struct Point3D : public Widget
{
  Point3D(const std::string & name, const ElementId & requestId);

  ~Point3D() override = default;

  void data(bool ro, const Eigen::Vector3d & pos, const mc_rtc::gui::PointConfig & config);

  void update(Client & client, SceneState & state) override;

  void draw3D(Camera camera) override;

private:
  ElementId requestId_;
  std::unique_ptr<InteractiveMarker> marker_;
};
