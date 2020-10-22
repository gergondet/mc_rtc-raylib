#include <mc_rbdyn/RobotLoader.h>
#include <mc_rbdyn/Robots.h>

#include <cmath>
#include <iostream>

#include "Camera.h"
#include "Client.h"
#include "utils.h"

int main(void)
{
  // Initialization
  //--------------------------------------------------------------------------------------
  const int screenWidth = 1600 / 2;
  const int screenHeight = 900 / 2;

  SetConfigFlags(FLAG_MSAA_4X_HINT);  // Enable Multi Sampling Anti Aliasing 4x (if available)
  InitWindow(screenWidth, screenHeight, "mc_rtc - raylib based 3D GUI");

  // Define the camera to look into our 3d world
  OrbitCamera camera;
  camera.position = (Vector3){ 3.0f, 3.0f, 3.0f }; // Camera position
  camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };    // Camera looking at point
  camera.up = (Vector3){ 0.0f, 0.0f, 1.0f };      // Camera up vector (rotation towards target)
  camera.fovy = 45.0f;                // Camera field-of-view Y
  camera.type = CAMERA_PERSPECTIVE;           // Camera mode type

  Client client("ipc:///tmp/mc_rtc_pub.ipc", "ipc:///tmp/mc_rtc_rep.ipc", 1);

  SetCameraMode(camera, CAMERA_FREE); // Set a free camera mode

  SetTargetFPS(60);           // Set our game to run at 60 frames-per-second

  SceneState state;
  state.camera = &camera;

  // Main game loop
  while (!WindowShouldClose())    // Detect window close button or ESC key
  {
    client.update(state);
    camera.update(state);
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

      ClearBackground(RAYWHITE);

      BeginMode3D(camera);

        DrawGridXY(10, 1.0f);

        DrawGizmo({0, 0, 0});

        client.draw3D(camera);

      EndMode3D();

      DrawFPS(10, 10);

    EndDrawing();
    //----------------------------------------------------------------------------------
  }

  client.clear();

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow();    // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}
