#pragma once

#include <mc_control/ControllerClient.h>

#include <mc_rbdyn/Robot.h>

#include <memory>
#include <string>

#include "../SceneState.h"

struct Client;

using ElementId = mc_control::ElementId;

/** A widget in the GUI */
struct Widget
{
  inline Widget(const ElementId & id) : id(id) {}

  virtual ~Widget() = default;

  ElementId id;
  bool seen = true;

  /** Update based on user interaction */
  virtual void update(Client &, SceneState &) {}

  /** Draw the 2D elements of the widget */
  virtual void draw2D() {}

  /** Draw the 3D elements of the widget */
  virtual void draw3D(Camera) {}
};

using WidgetPtr = std::unique_ptr<Widget>;

#include "../Client.h"
#include "imgui.h"
