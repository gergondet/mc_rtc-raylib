#pragma once

#include <mc_control/ControllerClient.h>

#include <mc_rbdyn/Robots.h>

#include "InteractiveMarker.h"
#include "RobotModel.h"
#include "SceneState.h"

struct Client : public mc_control::ControllerClient
{
  /* Typedef for brievity */
  using ElementId = mc_control::ElementId;

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

  void robot(const ElementId & id,
             const std::vector<std::string> & params,
             const std::vector<std::vector<double>> & q) override;

  void point3d(const ElementId & id,
               const ElementId & requestId,
               bool ro,
               const Eigen::Vector3d & pos,
               const mc_rtc::gui::PointConfig & config) override;

  void stopped() override;

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

  /** Category, the root has depth -1 */
  struct Category
  {
    Category() = default;
    inline Category(const std::string & name, int depth) : name(name), depth(depth) {}

    std::string name = "";
    int depth = -1;
    std::vector<WidgetPtr> widgets;
    std::vector<std::unique_ptr<Category>> categories;

    void update(Client & client, SceneState & state);
    void draw2D();
    void draw3D(Camera camera);
    void started();
    void stopped();
  };
  Category root_;

  /** Returns a category (creates it if it does not exist */
  Category & getCategory(const std::vector<std::string> & category);

  /** Get a widget with the right type and id */
  template<typename T, typename... Args>
  T & widget(const ElementId & id, Args &&... args)
  {
    auto & category = getCategory(id.category);
    auto it =
        std::find_if(category.widgets.begin(), category.widgets.end(), [&](auto & w) { return w->name == id.name; });
    if(it == category.widgets.end())
    {
      auto & w = category.widgets.emplace_back(std::make_unique<T>(id.name, std::forward<Args>(args)...));
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

  struct Point3D : public Widget
  {
    Point3D(const std::string & name, const ElementId & requestId);

    ~Point3D() override = default;

    void data(bool ro, const Eigen::Vector3d & pos, const mc_rtc::gui::PointConfig & config);

    void update(Client & client, SceneState & state) override;

    void draw3D(Camera camera) override;

  private:
    ElementId requestId_;
    std::unique_ptr<InteractiveMarker> marker_;
  };

  struct Robot : public Widget
  {
    Robot(const std::string & name, const std::vector<std::string> & parameters);

    ~Robot() override = default;

    void data(const std::vector<std::vector<double>> & q);

    void draw3D(Camera camera) override;

  private:
    std::shared_ptr<mc_rbdyn::Robots> robots_;
    std::unique_ptr<RobotModel> model_;
  };
};

