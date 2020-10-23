#pragma once

#include "SceneState.h"

struct OrbitCamera : public Camera
{
  void update(SceneState & state);

private:
  bool rotate = false;
  bool pan = false;
  Vector2 start;
};
