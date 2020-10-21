#pragma once

#include <SpaceVecAlg/SpaceVecAlg>

#include "raylib.h"
#include "raymath.h"
#include "SceneState.h"

struct InteractiveMarker
{
  InteractiveMarker(const sva::PTransformd & pose);

  void pose(const sva::PTransformd & pose);

  void update(Camera camera, Ray ray, SceneState & state);

  void draw();
private:
  sva::PTransformd pose_;
  float size = 0.05;
  struct MarkerPositionControl
  {
    MarkerPositionControl() = default;
    MarkerPositionControl(Color c, Eigen::Vector3d offset);
    bool active = false;
    Vector2 start;
    Vector2 dir;
    bool hover = false;
    Color color;
    Vector3 pose;
    BoundingBox bbox;
    Eigen::Vector3d offset;
  };
  std::array<MarkerPositionControl, 6> posControls_;
  struct MarkerOrientationControl
  {
    MarkerOrientationControl() = default;
    MarkerOrientationControl(Model torus, Color c, Vector3 normal, Matrix orientation = MatrixIdentity());
    Model torus;
    Color color;
    Vector3 normal;
    Matrix orientation;
    bool hover = false;
    bool active = false;
    Vector3 pZ;
    Vector3 pX;
    Vector3 pY;
  };
  std::array<MarkerOrientationControl, 3> oriControls_;
};
