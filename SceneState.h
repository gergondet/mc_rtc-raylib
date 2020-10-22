#pragma once

#include "raylib.h"

/** Represent the scene state */
struct SceneState
{
  /** 3D Camera used in the scene */
  Camera * camera;
  /** Point to the current handler of mouse motions */
  void * mouseHandler = nullptr;
};
