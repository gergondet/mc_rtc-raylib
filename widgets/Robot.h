#pragma once

#include "../win32_defs.h"

#include "Widget.h"

struct Robot : public Widget
{
  Robot(Client & client, const ElementId & id, const std::vector<std::string> & parameters);

  ~Robot() override = default;

  void data(const std::vector<std::vector<double>> & q, const sva::PTransformd & posW);

  void draw2D() override;

  void draw3D(Camera camera) override;

private:
  std::shared_ptr<mc_rbdyn::Robots> robots_;
  std::unique_ptr<RobotModel> model_;
  std::unique_ptr<RobotModel> collisionModel_;
  bool display_ = true;
  bool displayCollision_ = false;
};
