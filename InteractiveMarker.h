#pragma once

#include <SpaceVecAlg/SpaceVecAlg>

#include "SceneState.h"
#include "raylib.h"
#include "raymath.h"

#include <memory>

enum class ControlAxis
{
  TX = (1u << 0),
  TY = (1u << 1),
  TZ = (1u << 2),
  RX = (1u << 3),
  RY = (1u << 4),
  RZ = (1u << 5),
  TRANSLATION = TX | TY | TZ,
  ROTATION = RX | RY | RZ,
  XYTHETA = TX | TY | RZ,
  ALL = TRANSLATION | ROTATION
};

inline ControlAxis operator|(ControlAxis lhs, ControlAxis rhs)
{
  using enum_t = std::underlying_type_t<ControlAxis>;
  return static_cast<ControlAxis>(static_cast<enum_t>(lhs) | static_cast<enum_t>(rhs));
}

inline ControlAxis operator&(ControlAxis lhs, ControlAxis rhs)
{
  using enum_t = std::underlying_type_t<ControlAxis>;
  return static_cast<ControlAxis>(static_cast<enum_t>(lhs) & static_cast<enum_t>(rhs));
}

struct InteractiveMarker
{
  InteractiveMarker(const sva::PTransformd & pose, ControlAxis mask = ControlAxis::ALL);

  void pose(const sva::PTransformd & pose);

  inline const sva::PTransformd & pose() const noexcept
  {
    return pose_;
  }

  void update(SceneState & state);

  void draw();

  inline bool active()
  {
    return active_;
  }

private:
  bool active_ = false;
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
  std::vector<MarkerPositionControl> posControls_;
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
  std::vector<MarkerOrientationControl> oriControls_;
};

using InteractiveMarkerPtr = std::unique_ptr<InteractiveMarker>;
