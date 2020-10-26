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

  inline const mc_rtc::Configuration & data() const noexcept
  {
    return data_;
  }

private:
  std::vector<char> buffer_ = std::vector<char>(65535);
  std::chrono::system_clock::time_point t_last_ = std::chrono::system_clock::now();

  /** No message for unsupported types */
  void default_impl(const std::string &, const ElementId &) final {}

  void started() override;

  void category(const std::vector<std::string> &, const std::string &) override;

  void label(const ElementId & id, const std::string & txt) override;

  void array_label(const ElementId & id,
                   const std::vector<std::string> & labels,
                   const Eigen::VectorXd & data) override;

  void button(const ElementId & id) override;

  void checkbox(const ElementId & id, bool state) override;

  void string_input(const ElementId & id, const std::string & data) override;

  void integer_input(const ElementId & id, int data) override;

  void number_input(const ElementId & id, double data) override;

  void number_slider(const ElementId & id, double data, double min, double max) override;

  void array_input(const ElementId & id,
                   const std::vector<std::string> & labels,
                   const Eigen::VectorXd & data) override;

  void combo_input(const ElementId & id, const std::vector<std::string> & values, const std::string & data) override;

  void data_combo_input(const ElementId & id, const std::vector<std::string> & ref, const std::string & data) override;

  void point3d(const ElementId & id,
               const ElementId & requestId,
               bool ro,
               const Eigen::Vector3d & pos,
               const mc_rtc::gui::PointConfig & config) override;

  void trajectory(const ElementId & id,
                  const std::vector<Eigen::Vector3d> & points,
                  const mc_rtc::gui::LineConfig & config) override;

  void trajectory(const ElementId & id,
                  const std::vector<sva::PTransformd> & points,
                  const mc_rtc::gui::LineConfig & config) override;

  void trajectory(const ElementId & id, const Eigen::Vector3d & point, const mc_rtc::gui::LineConfig & config) override;

  void trajectory(const ElementId & id,
                  const sva::PTransformd & point,
                  const mc_rtc::gui::LineConfig & config) override;

  void polygon(const ElementId & id,
               const std::vector<std::vector<Eigen::Vector3d>> & points,
               const mc_rtc::gui::Color & color) override;

  void force(const ElementId & id,
             const ElementId & requestId,
             const sva::ForceVecd & force,
             const sva::PTransformd & pos,
             const mc_rtc::gui::ForceConfig & forceConfig,
             bool /* ro */) override;

  void arrow(const ElementId & id,
             const ElementId & requestId,
             const Eigen::Vector3d & start,
             const Eigen::Vector3d & end,
             const mc_rtc::gui::ArrowConfig & config,
             bool ro) override;

  void rotation(const ElementId & id, const ElementId & requestId, bool ro, const sva::PTransformd & pos) override;

  void robot(const ElementId & id,
             const std::vector<std::string> & params,
             const std::vector<std::vector<double>> & q) override;

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
      auto & w = category.widgets.emplace_back(std::make_unique<T>(*this, id, std::forward<Args>(args)...));
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
