#pragma once

#include <mc_rbdyn/Robot.h>

#include "raylib.h"

struct RobotModel
{

  RobotModel(const mc_rbdyn::Robot & robot, bool useCollisionModel = false);

  void draw(const mc_rbdyn::Robot & robot, Camera camera, Ray ray);

private:
  using draw_call_t = std::function<void(const sva::PTransformd&, Ray)>;
  std::vector<draw_call_t> draw_;
  Shader shader_;
};
