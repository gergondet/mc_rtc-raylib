#pragma once

#include <mc_rbdyn/Robot.h>
#include "../SceneState.h"

#include <memory>
#include <string>
#include <vector>

struct Client;

struct Widget;
using WidgetPtr = std::unique_ptr<Widget>;

struct Category;
using CategoryPtr = std::unique_ptr<Category>;

/** Category, the root has depth -1 */
struct Category
{
  Category() = default;
  inline Category(const std::string & name, int depth) : name(name), depth(depth) {}

  std::string name = "";
  int depth = -1;
  std::vector<WidgetPtr> widgets;
  std::vector<CategoryPtr> categories;

  void update(Client & client, SceneState & state);
  void draw2D();
  void draw3D(Camera camera);
  void started();
  void stopped();
};

#include "Widget.h"
