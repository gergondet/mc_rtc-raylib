#pragma once

#include "raylib.h"

struct OrbitCamera : public Camera
{
  void update();
private:
  bool rotate = false;
  bool pan = false;
  Vector2 start;
};
