#pragma once

#include "raylib.h"

/** Represent the scene state */
struct SceneState
{
  /** Point to the current handler of mouse motions */
  void * mouseHandler = nullptr;
};
