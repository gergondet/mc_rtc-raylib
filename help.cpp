#include "help.h"

#include "imgui.h"
#include "raylib.h"

inline void ShowBulletPoints(std::initializer_list<const char *> strings)
{
  for(const auto & s : strings)
  {
    ImGui::Bullet();
    ImGui::TextWrapped("%s", s);
  }
}

inline void ShowLIPMWalkingHelp()
{
  if(ImGui::BeginTabItem("LIPMWalking"))
  {
    // clang-format off
    ShowBulletPoints({
      "Click the \"Start standing\" button to start the stabilizer",
      "Move the marker on the ground to set the destination or choose a pre-defined plan",
      "The marker is only available when a custom_* plan is selected",
      "Click on \"Start walking\" to execute the plan",
      "This controller is only compatible with humanoid robots"
    });
    // clang-format on
    ImGui::EndTabItem();
  }
}

inline void ShowGeneralHelp()
{
  if(ImGui::BeginTabItem("General"))
  {
    // clang-format off
    ShowBulletPoints({
      "The \"Tasks\" tab lets you edit the active tasks properties",
      "You can also interact with 3D markers to change some tasks objectives",
      "You can also remove tasks from there",
      "The \"Global/Add tasks\" tab lets you add tasks into the controller, you should first select a task from the dropdown menu then fill the required and optional parameters as you see fit",
      "Some tasks will not work correctly in this demo (e.g. force tasks)"
    });
    // clang-format on
    ImGui::EndTabItem();
  }
}

inline void ShowTickerHelp()
{
  if(ImGui::BeginTabItem("Ticker"))
  {
    // clang-format off
    ShowBulletPoints({
      "Click the \"Stop\" button to stop the controller",
      "You can then select a different robot, controller and timestep to run",
#ifdef __EMSCRIPTEN__
      "You can also edit the robot and controller parameters in the address bar for the same effect, this can also be used as a permanent link to a specific robot/controller combination",
#endif
      "The combination of robot/controller/timestep might fail, if so an error message should be displayed in the \"Console\" window"
    });
    // clang-format on
    ImGui::EndTabItem();
  }
}

void ShowHelpWindow(bool & show, const std::string & controller)
{
  auto left_margin = 15;
  auto right_margin = 15;
  auto top_margin = 50;
  auto bottom_margin = 50;
  auto width = GetScreenWidth() - left_margin - right_margin;
  auto height = GetScreenHeight() - top_margin - bottom_margin;
  auto w_width = 0.3 * width;
  auto w_height = 0.45 * height;
  ImGui::SetNextWindowPos(ImVec2(left_margin + width - w_width, top_margin), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(w_width, w_height), ImGuiCond_FirstUseEver);
  if(show && ImGui::Begin("Help", &show))
  {
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if(ImGui::BeginTabBar("#HelpTabs", tab_bar_flags))
    {
      if(controller == "LIPMWalking")
      {
        ShowLIPMWalkingHelp();
      }
      ShowGeneralHelp();
      ShowTickerHelp();
      ImGui::EndTabBar();
    }
    ImGui::End();
  }
}
