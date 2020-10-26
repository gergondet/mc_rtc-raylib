#pragma once

#include <mc_control/ControllerClient.h>

#include <mc_rbdyn/Robot.h>

#include <memory>
#include <string>

#include "../SceneState.h"

#ifdef SPDLOG_FMT_EXTERNAL
#  include <fmt/ranges.h>
#else
#  include <spdlog/fmt/bundled/ranges.h>
#endif

struct Client;

using ElementId = mc_control::ElementId;

/** A widget in the GUI */
struct Widget
{
  inline Widget(Client & client, const ElementId & id) : client(client), id(id) {}

  virtual ~Widget() = default;

  Client & client;
  ElementId id;
  bool seen = true;

  /** Draw the 2D elements of the widget */
  virtual void draw2D() {}

  /** Update 3D objects based on user interaction */
  virtual void update(SceneState &) {}

  /** Draw the 3D elements of the widget */
  virtual void draw3D(Camera) {}

  inline std::string label(std::string_view label, std::string_view extra = "")
  {
    return fmt::format("{}##{}{}{}", label, id.category, id.name, extra);
  }

  inline std::string label(std::string_view label, int i)
  {
    return fmt::format("{}##{}{}{}", label, id.category, id.name, i);
  }

  inline std::string label(std::string_view label, size_t i)
  {
    return fmt::format("{}##{}{}{}", label, id.category, id.name, i);
  }
};

using WidgetPtr = std::unique_ptr<Widget>;

#include "../Client.h"
#include "imgui.h"
