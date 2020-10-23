#include <mc_rbdyn/RobotLoader.h>
#include <mc_rbdyn/Robots.h>

#include <cmath>
#include <iostream>

#include "Camera.h"
#include "Client.h"
#include "utils.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_raylib.h"

int main(void)
{
  // Initialization
  //--------------------------------------------------------------------------------------
  const int screenWidth = 1600;
  const int screenHeight = 900;

  SetConfigFlags(FLAG_MSAA_4X_HINT);  // Enable Multi Sampling Anti Aliasing 4x (if available)
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  SetTraceLogLevel(LOG_WARNING);
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

  ImGui::CreateContext();
  ImGuiIO & io = ImGui::GetIO();
  ImGui::StyleColorsDark();

  ImGui_ImplOpenGL3_Init();
  ImGui_ImplRaylib_Init();

  SceneState state;
  state.camera = &camera;

  // Main game loop
  while (!WindowShouldClose())    // Detect window close button or ESC key
  {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplRaylib_NewFrame();
    ImGui::NewFrame();
    ImGui_ImplRaylib_ProcessEvent();

    if(io.WantCaptureMouse && state.mouseHandler == nullptr)
    {
      state.mouseHandler = &io;
    }
    else if(!io.WantCaptureMouse && state.mouseHandler == &io)
    {
      state.mouseHandler = nullptr;
    }

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

      client.draw2D();
      ImGui::ShowDemoWindow();
      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    EndDrawing();
    //----------------------------------------------------------------------------------
  }

  client.clear();

  ImGui_ImplRaylib_Shutdown();
  ImGui_ImplOpenGL3_Shutdown();

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow();    // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}
