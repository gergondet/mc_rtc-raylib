#include <mc_rbdyn/RobotLoader.h>
#include <mc_rbdyn/Robots.h>

#include <cmath>
#include <iostream>

#include "Camera.h"
#include "Client.h"
#include "utils.h"

#include "Robot_Regular_ttf.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_raylib.h"

int main(void)
{
  // Initialization
  //--------------------------------------------------------------------------------------
  const int screenWidth = 1600;
  const int screenHeight = 900;

  SetConfigFlags(FLAG_MSAA_4X_HINT); // Enable Multi Sampling Anti Aliasing 4x (if available)
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  SetTraceLogLevel(LOG_WARNING);
  InitWindow(screenWidth, screenHeight, "mc_rtc - raylib based 3D GUI");

  // Define the camera to look into our 3d world
  OrbitCamera camera;
  camera.position = (Vector3){2.5f, -1.5f, 1.3f}; // Camera position
  camera.target = (Vector3){-0.15f, -0.4f, 0.75f}; // Camera looking at point
  camera.up = (Vector3){0.0f, 0.0f, 1.0f}; // Camera up vector (rotation towards target)
  camera.fovy = 60.0f; // Camera field-of-view Y
  camera.type = CAMERA_PERSPECTIVE; // Camera mode type

  Client client("ipc:///tmp/mc_rtc_pub.ipc", "ipc:///tmp/mc_rtc_rep.ipc", 1);

  SetCameraMode(camera, CAMERA_FREE); // Set a free camera mode

  SetTargetFPS(60); // Set our game to run at 60 frames-per-second

  ImGui::CreateContext();
  ImGuiIO & io = ImGui::GetIO();
  ImGui::StyleColorsLight();
  io.FontDefault = io.Fonts->AddFontFromMemoryTTF(Roboto_Regular_ttf, Roboto_Regular_ttf_len, 18.0f);
  auto & style = ImGui::GetStyle();
  style.FrameRounding = 6.0f;
  auto & bgColor = style.Colors[ImGuiCol_WindowBg];
  bgColor.w = 0.5f;

  ImGui_ImplOpenGL3_Init();
  ImGui_ImplRaylib_Init();

  SceneState state;
  state.camera = &camera;

  // Main game loop
  while(!WindowShouldClose()) // Detect window close button or ESC key
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

    client.draw3D(camera);

    DrawFrame(sva::PTransformd::Identity());

    EndMode3D();

    DrawFPS(10, 10);

    client.draw2D();
    ImGui::ShowDemoWindow();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    EndDrawing();
    //----------------------------------------------------------------------------------
  }

  mc_rtc::log::info("Camera on exit");
  mc_rtc::log::info("position: {}, {}, {}", camera.position.x, camera.position.y, camera.position.z);
  mc_rtc::log::info("target: {}, {}, {}", camera.target.x, camera.target.y, camera.target.z);
  mc_rtc::log::info("up: {}, {}, {}", camera.up.x, camera.up.y, camera.up.z);
  mc_rtc::log::info("fovy: {}", camera.fovy);

  client.clear();

  ImGui_ImplRaylib_Shutdown();
  ImGui_ImplOpenGL3_Shutdown();

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow(); // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}
