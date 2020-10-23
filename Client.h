#pragma once

#include <mc_control/ControllerClient.h>

#include <mc_rbdyn/Robots.h>

#include "InteractiveMarker.h"
#include "RobotModel.h"
#include "SceneState.h"

#include "widgets/Category.h"

/* Typedef for brievity */
using ElementId = mc_control::ElementId;

struct Client : public mc_control::ControllerClient
{
  /** Same constructors as the base class */
  using mc_control::ControllerClient::ControllerClient;

  /** Update the client data from the latest server message */
  void update(SceneState & state);

  /** Draw 2D elements */
  void draw2D();

  /** Draw 3D elements */
  void draw3D(Camera camera);

  /** Remove all elements */
  void clear();

private:
  std::vector<char> buffer_ = std::vector<char>(65535);
  std::chrono::system_clock::time_point t_last_ = std::chrono::system_clock::now();

  /** No message for unsupported types */
  void default_impl(const std::string &, const ElementId &) final {}

  void started() override;

  void category(const std::vector<std::string> &, const std::string &) override;

  void label(const ElementId & id, const std::string & txt) override;

  void checkbox(const ElementId & id, bool state) override;

  void string_input(const ElementId & id, const std::string & data) override;

  void array_label(const ElementId & id,
                   const std::vector<std::string> & labels,
                   const Eigen::VectorXd & data) override;

  void button(const ElementId & id) override;

  void robot(const ElementId & id,
             const std::vector<std::string> & params,
             const std::vector<std::vector<double>> & q) override;

  void point3d(const ElementId & id,
               const ElementId & requestId,
               bool ro,
               const Eigen::Vector3d & pos,
               const mc_rtc::gui::PointConfig & config) override;

  void stopped() override;

  Category root_;

  /** Returns a category (creates it if it does not exist */
  Category & getCategory(const std::vector<std::string> & category);

  /** Get a widget with the right type and id */
  template<typename T, typename... Args>
  T & widget(const ElementId & id, Args &&... args)
  {
    auto & category = getCategory(id.category);
    auto it =
        std::find_if(category.widgets.begin(), category.widgets.end(), [&](auto & w) { return w->id.name == id.name; });
    if(it == category.widgets.end())
    {
      auto & w = category.widgets.emplace_back(std::make_unique<T>(id, std::forward<Args>(args)...));
      w->seen = true;
      return *dynamic_cast<T *>(w.get());
    }
    else
    {
      auto w_ptr = dynamic_cast<T *>(it->get());
      if(w_ptr)
      {
        w_ptr->seen = true;
        return *w_ptr;
      }
      /** Different type, remove and add the widget again */
      category.widgets.erase(it);
      return widget<T>(id, std::forward<Args>(args)...);
    }
  }
};

