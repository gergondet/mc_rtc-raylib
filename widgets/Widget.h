#pragma once

#include <mc_rbdyn/Robot.h>

#include <memory>
#include <string>

#include "../SceneState.h"

struct Client;

/** A widget in the GUI */
struct Widget
{
  inline Widget(const std::string & name) : name(name) {}

  virtual ~Widget() = default;

  std::string name;
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
