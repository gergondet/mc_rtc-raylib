#pragma once

#include "Widget.h"

struct Robot : public Widget
{
  Robot(Client & client, const ElementId & id);

  ~Robot() override = default;

  void data(const std::vector<std::string> & params, const std::vector<std::vector<double>> & q, const sva::PTransformd & posW);

  void draw2D() override;

  void draw3D(Camera camera) override;

private:
  void reset(const std::vector<std::string> & params);

  std::vector<std::string> params_;
  std::shared_ptr<mc_rbdyn::Robots> robots_;
  std::unique_ptr<RobotModel> model_;
  std::unique_ptr<RobotModel> collisionModel_;
  bool display_ = true;
  bool displayCollision_ = false;
};
