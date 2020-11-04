#include <mc_rbdyn/RobotLoader.h>
#include <mc_rbdyn/Robots.h>

#include <mc_rtc/version.h>

#include <mc_control/mc_global_controller.h>

#include <cmath>
#include <iostream>

#include "Camera.h"
#include "Client.h"
#include "utils.h"

#include "Robot_Regular_ttf.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_raylib.h"

#ifdef __EMSCRIPTEN__
#  include <emscripten/emscripten.h>
#endif

// Target FPS for the ticker
size_t fps = 50;

static OrbitCamera * camera_ptr = nullptr;
static Client * client_ptr = nullptr;
static SceneState * state_ptr = nullptr;
static ImGuiIO * io_ptr = nullptr;

#ifdef __EMSCRIPTEN__
static bool with_ticker = true;
#else
static bool with_ticker = true;
#endif

struct TickerLoopData
{
  std::thread thread;
  bool running = true;
  size_t iter = 0;
  mc_control::MCGlobalController * gc = nullptr;
  std::function<void()> simulateSensors;
  std::chrono::system_clock::time_point start_t;
};

static TickerLoopData data;

void StartTicker()
{
  data.thread = std::thread([]() {
    data.running = true;
    mc_control::MCGlobalController gc("/assets/etc/mc_rtc.yaml");

    const auto & mb = gc.robot().mb();
    const auto & mbc = gc.robot().mbc();
    const auto & rjo = gc.ref_joint_order();
    std::vector<double> initq;
    for(const auto & jn : rjo)
    {
      for(const auto & qi : mbc.q[static_cast<unsigned int>(mb.jointIndexByName(jn))])
      {
        initq.push_back(qi);
      }
    }

    std::vector<double> qEnc(initq.size(), 0);
    std::vector<double> alphaEnc(initq.size(), 0);
    auto simulateSensors = [&, qEnc, alphaEnc]() mutable {
      auto & robot = gc.robot();
      for(unsigned i = 0; i < robot.refJointOrder().size(); i++)
      {
        auto jIdx = robot.jointIndexInMBC(i);
        if(jIdx != -1)
        {
          auto jointIndex = static_cast<unsigned>(jIdx);
          qEnc[i] = robot.mbc().q[jointIndex][0];
          alphaEnc[i] = robot.mbc().alpha[jointIndex][0];
        }
      }
      gc.setEncoderValues(qEnc);
      gc.setEncoderVelocities(alphaEnc);
      gc.setSensorPositions({{"FloatingBase", robot.posW().translation()}});
      gc.setSensorOrientations({{"FloatingBase", Eigen::Quaterniond{robot.posW().rotation()}}});
    };
    data.simulateSensors = simulateSensors;

    gc.setEncoderValues(qEnc);
    gc.init(initq, gc.robot().module().default_attitude());
    gc.running = true;

    auto loop_fn = [](void * dataPtr) {
      auto & data = *static_cast<TickerLoopData *>(dataPtr);
      if(!data.running)
      {
        client_ptr->stop();
        data.gc = nullptr;
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
#endif
        return;
      }
      data.simulateSensors();
      data.gc->run();
      if(++data.iter % fps == 0)
      {
        using duration_ms = std::chrono::duration<double, std::milli>;
        double dt = duration_ms(std::chrono::system_clock::now() - data.start_t).count();
        mc_rtc::log::info("Controller running for {} seconds, real-time: {}", data.iter * data.gc->timestep(),
                          dt / 1000.0);
      }
    };

    data.iter = 0;
    data.gc = &gc;
    data.start_t = std::chrono::system_clock::now();
    client_ptr->connect(gc.server(), *gc.controller().gui());
    fps = static_cast<int>(1 / gc.timestep());
    mc_rtc::log::info("Target background fps: {}", fps);
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(loop_fn, &data, fps, 1);
#else
    while(data.running)
    {
      auto now = std::chrono::system_clock::now();
      loop_fn(&data);
      std::this_thread::sleep_until(now + std::chrono::milliseconds(static_cast<int>(1000 * gc.timestep())));
    }
    client_ptr->stop();
    data.gc = nullptr;
#endif
  });
#ifdef __EMSCRIPTEN__
  data.thread.detach();
#endif
}

void EmptyRender()
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplRaylib_NewFrame();
  ImGui::NewFrame();
  ImGui_ImplRaylib_ProcessEvent();
  BeginDrawing();
  ClearBackground(RAYWHITE);
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  EndDrawing();
}

void RenderLoop()
{
  static auto & camera = *camera_ptr;
  static auto & client = *client_ptr;
  static auto & state = *state_ptr;
  static auto & io = *io_ptr;
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplRaylib_NewFrame();
  ImGui::NewFrame();
  ImGui_ImplRaylib_ProcessEvent();

#ifdef __EMSCRIPTEN__
  ShowCursor();
#endif

  if(io.WantCaptureMouse && state.mouseHandler == nullptr)
  {
    state.mouseHandler = &io;
  }
  else if(!io.WantCaptureMouse && state.mouseHandler == &io)
  {
    state.mouseHandler = nullptr;
  }

  if(with_ticker)
  {
    if(data.running)
    {
      client.update(state);
    }
    else
    {
      client.clear();
    }
  }
  else
  {
    client.update(state);
  }
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
  if(with_ticker)
  {
    auto left_margin = 15;
    auto right_margin = 15;
    auto top_margin = 50;
    auto bottom_margin = 50;
    auto width = GetScreenWidth() - left_margin - right_margin;
    auto height = GetScreenHeight() - top_margin - bottom_margin;
    auto w_width = 0.05 * width;
    auto w_height = 0.1 * height;
    ImGui::SetNextWindowPos(ImVec2(width + left_margin - w_width, height + top_margin - w_height),
                            ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(w_width, w_height), ImGuiCond_FirstUseEver);
    ImGui::Begin("Ticker");
    bool starting = data.gc == nullptr;
    bool running = data.running;
    if(running)
    {
      if(starting)
      {
        ImGui::Text("Controller starting up");
      }
      else
      {
        if(ImGui::Button("Stop"))
        {
          data.running = false;
          if(data.thread.joinable())
          {
            data.thread.join();
          }
        }
      }
    }
    else
    {
      if(ImGui::Button("Start"))
      {
        StartTicker();
      }
    }
    ImGui::End();
  }
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  EndDrawing();
  //----------------------------------------------------------------------------------
}

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
  camera_ptr = &camera;
  camera.position = (Vector3){2.5f, -1.5f, 1.3f}; // Camera position
  camera.target = (Vector3){-0.15f, -0.4f, 0.75f}; // Camera looking at point
  camera.up = (Vector3){0.0f, 0.0f, 1.0f}; // Camera up vector (rotation towards target)
  camera.fovy = 60.0f; // Camera field-of-view Y
  camera.type = CAMERA_PERSPECTIVE; // Camera mode type

  Client client;
  client.register_log_sink();
  client_ptr = &client;

  SetCameraMode(camera, CAMERA_FREE); // Set a free camera mode

  SetTargetFPS(60); // Set our game to run at 60 frames-per-second

  ImGui::CreateContext();
  ImGuiIO & io = ImGui::GetIO();
  io_ptr = &io;
  ImGui::StyleColorsLight();
  io.FontDefault = io.Fonts->AddFontFromMemoryTTF(Roboto_Regular_ttf, Roboto_Regular_ttf_len, 18.0f);
  auto & style = ImGui::GetStyle();
  style.FrameRounding = 6.0f;
  auto & bgColor = style.Colors[ImGuiCol_WindowBg];
  bgColor.w = 0.5f;

  ImGui_ImplOpenGL3_Init();
  ImGui_ImplRaylib_Init();

  SceneState state;
  state_ptr = &state;
  state.camera = &camera;

  mc_rtc::log::info("mc_rtc::version() {}", mc_rtc::version());

  if(with_ticker)
  {
    StartTicker();
  }
  else
  {
    client.connect("ipc:///tmp/mc_rtc_pub.ipc", "ipc:///tmp/mc_rtc_rep.ipc");
    client.timeout(1.0);
  }

  // Make sure the window appears once before we start anything else
  EmptyRender();
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(RenderLoop, 0, 1);
#else
  while(!WindowShouldClose())
  {
    RenderLoop();
  }
#endif

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

  if(data.thread.joinable())
  {
    data.running = false;
    data.thread.join();
  }

  return 0;
}
