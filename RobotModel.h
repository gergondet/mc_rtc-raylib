#pragma once

#include <mc_rbdyn/Robot.h>

#include "raylib.h"

struct BodyDrawer
{
  BodyDrawer(const std::vector<rbd::parsers::Visual> & v, Shader * shader = nullptr);

  void update(const sva::PTransformd & pos);

  void draw();

private:
  struct ModelData
  {
    Model model;
    float scale;
    Color color;
    sva::PTransformd X_b_model;
    ~ModelData();
  };
  std::vector<std::unique_ptr<ModelData>> models_;
};

struct RobotModel
{
  RobotModel(const mc_rbdyn::Robot & robot, bool useCollisionModel = false);

  ~RobotModel();

  void update(const mc_rbdyn::Robot & robot);

  void draw(Camera camera);

private:
  std::vector<BodyDrawer> bodies_;
  Shader shader_;
};
