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

#include <boost/filesystem.hpp>
namespace bfs = boost::filesystem;

/** mc_rbdyn::RobotLoader::available_robots() but hides env, object, json and related aliases */
static std::vector<std::string> get_available_robots()
{
  auto out = mc_rbdyn::RobotLoader::available_robots();
  out.erase(std::remove_if(out.begin(), out.end(),
                           [](const auto & s) {
                             return s == "env" || s == "object" || s == "json"
                                    || (s.size() >= 4 && s.substr(0, 4) == "env/")
                                    || (s.size() >= 7 && s.substr(0, 7) == "object/");
                           }),
            out.end());
  std::sort(out.begin(), out.end());
  return out;
}

// Target FPS for the ticker
size_t fps = 50;

// Override target FPS for some known controllers
std::unordered_map<std::string, size_t> controllers_fps = {{"AdmittanceSample", 200}, {"LIPMStabilizer", 200}};

static OrbitCamera * camera_ptr = nullptr;
static Client * client_ptr = nullptr;
static SceneState * state_ptr = nullptr;
static ImGuiIO * io_ptr = nullptr;

#ifdef __EMSCRIPTEN__
static bool with_ticker = true;
#else
static bool with_ticker = true;
#endif

struct TickerConfiguration
{
  bfs::path directory;
  mc_rtc::Configuration config;
  std::string MainRobot;
  std::string Enabled;
  std::vector<std::string> robots;
  std::vector<std::string> controllers;
};

struct TickerLoopData
{
  std::thread thread;
  bool running = true;
  size_t iter = 0;
  std::unique_ptr<mc_control::MCGlobalController> gc = nullptr;
  std::function<void()> simulateSensors;
  std::chrono::system_clock::time_point start_t;
  double ratio = 1.0;
  TickerConfiguration config;
  size_t display = 0;
};

static TickerLoopData data;

void StartTicker()
{
  if(data.thread.joinable())
  {
    data.thread.join();
  }
  data.thread = std::thread([]() {
    try
    {

      data.running = true;
#ifdef __EMSCRIPTEN__
      {
        mc_rtc::Configuration config((data.config.directory / "mc_rtc.yaml").string());
        std::string next_ctl = config("Enabled");
        double dt = 1.0 / static_cast<double>(controllers_fps.count(next_ctl) ? controllers_fps.at(next_ctl) : fps);
        config.add("Timestep", dt);
        config.save((data.config.directory / "mc_rtc.yaml").string());
      }
#endif
      auto gc_ptr = std::make_unique<mc_control::MCGlobalController>((data.config.directory / "mc_rtc.yaml").string());
      auto & gc = *gc_ptr;
#ifndef __EMSCRIPTEN__
      fps = gc.timestep();
#endif
      data.config.config = gc.configuration().config;

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
        if(robot.hasBodySensor("FloatingBase"))
        {
          gc.setSensorPositions({{"FloatingBase", robot.posW().translation()}});
          gc.setSensorOrientations({{"FloatingBase", Eigen::Quaterniond{robot.posW().rotation()}}});
        }
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
        double sim_t = data.iter++ * data.gc->timestep();
        using duration_ms = std::chrono::duration<double, std::milli>;
        double real_t = duration_ms(std::chrono::system_clock::now() - data.start_t).count() / 1000.0;
        data.ratio = sim_t / real_t;
      };

      data.iter = 0;
      data.gc = std::move(gc_ptr);
      data.start_t = std::chrono::system_clock::now();
      client_ptr->connect(gc.server(), *gc.controller().gui());
      fps = static_cast<int>(1 / gc.timestep());
      mc_rtc::log::info("Target background fps: {}", fps);
#ifdef __EMSCRIPTEN__
      emscripten_set_main_loop_arg(loop_fn, &data, fps, 1);
#else
      while(data.running)
      {
        auto next =
            std::chrono::system_clock::now() + std::chrono::microseconds(static_cast<int>(1000 * 1000 * gc.timestep()));
        loop_fn(&data);
        std::this_thread::sleep_until(next);
      }
      client_ptr->stop();
      data.gc = nullptr;
#endif
    }
    catch(...)
    {
#ifndef __EMSCRIPTEN__
      client_ptr->stop();
#endif
      data.running = false;
      data.gc = nullptr;
    }
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

  state.startUpdate();
  if(io.WantCaptureMouse && !state.hasMouse(&io))
  {
    state.attemptMouseCapture(&io, -std::numeric_limits<float>::infinity(), []() { return true; });
  }
  else if(!io.WantCaptureMouse && state.hasMouse(&io))
  {
    state.releaseMouse(&io);
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
  state.endUpdate();
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

  if(data.running && data.gc == nullptr) // starting
  {
    std::string text = "Controller is starting, please wait...";
    auto tSize = MeasureText(text.c_str(), 20);
    size_t np = ((data.display % 60) * 4) / 60;
    text = text.substr(0, text.size() - 3) + std::string(np, '.');
    auto posX = (GetScreenWidth() - tSize) / 2;
    auto posY = 0.25 * GetScreenHeight();
    DrawText(text.c_str(), posX, posY, 20, DARKGRAY);
  }

  if(!with_ticker)
  {
    DrawFPS(10, 10);
  }
  else
  {
    DrawText(TextFormat("%2i FPS | %0.2f Sim/Real", GetFPS(), data.ratio), 10, 10, 20, LIME);
  }

  client.draw2D();
  if(with_ticker)
  {
    auto left_margin = 15;
    auto right_margin = 15;
    auto top_margin = 50;
    auto bottom_margin = 50;
    auto width = GetScreenWidth() - left_margin - right_margin;
    auto height = GetScreenHeight() - top_margin - bottom_margin;
    auto w_width = 0.2 * width;
    auto w_height = 0.2 * height;
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
          data.config.config = data.gc->configuration().config;
          data.config.MainRobot = static_cast<std::string>(data.config.config("MainRobot"));
          data.config.Enabled = static_cast<std::string>(data.gc->current_controller());
          data.config.robots = get_available_robots();
          data.config.controllers = data.gc->loaded_controllers();
          std::sort(data.config.controllers.begin(), data.config.controllers.end());
          data.running = false;
          data.ratio = 1.0;
          if(data.thread.joinable())
          {
            data.thread.join();
          }
        }
      }
    }
    else
    {
      Combo("Robot", data.config.robots, data.config.MainRobot);
      Combo("Controller", data.config.controllers, data.config.Enabled);
      static int fps_in = fps;
      auto flags = ImGuiInputTextFlags_None;
#ifdef __EMSCRIPTEN__
      if(controllers_fps.count(data.config.Enabled))
      {
        flags = ImGuiInputTextFlags_ReadOnly;
        fps_in = controllers_fps.at(data.config.Enabled);
      }
      else
      {
        fps_in = fps;
      }
#endif
      if(ImGui::InputInt("Ticker FPS", &fps_in, 0, 0, flags))
      {
        if(fps_in > 0)
        {
          fps = fps_in;
        }
      }
      if(ImGui::Button("Start"))
      {
        if(fps_in > 0)
        {
          fps = fps_in;
        }
        client_ptr->clearConsole();
        auto c_path = (data.config.directory / "mc_rtc.yaml").string();
        mc_rtc::Configuration config(c_path);
        config.add("MainRobot", data.config.MainRobot);
        config.add("Enabled", data.config.Enabled);
        double dt = 1.0 / static_cast<double>(fps);
        config.add("Timestep", dt);
        config.save(c_path);
        StartTicker();
      }
    }
    ImGui::End();
  }
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  EndDrawing();

  data.display++;
}

int main(void)
{
  // Initialization
  //--------------------------------------------------------------------------------------
  const int screenWidth = 1600;
  const int screenHeight = 900;

#ifndef __EMSCRIPTEN__
  data.config.directory = bfs::temp_directory_path() / bfs::unique_path();
  bfs::create_directories(data.config.directory);
#else
  data.config.directory = bfs::path("/assets/etc");
#endif

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

#ifndef __EMSCRIPTEN__
  bfs::remove_all(data.config.directory);
#endif

  return 0;
}
