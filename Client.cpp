#include "Client.h"

#include "widgets/ArrayLabel.h"
#include "widgets/Button.h"
#include "widgets/Checkbox.h"
#include "widgets/Label.h"
#include "widgets/Point3D.h"
#include "widgets/Robot.h"

void Client::update(SceneState & state)
{
  run(buffer_, t_last_);
  root_.update(*this, state);
}

void Client::draw2D()
{
  ImGui::Begin("mc_rtc");
  root_.draw2D();
  ImGui::End();
}

void Client::draw3D(Camera camera)
{
  root_.draw3D(camera);
}

void Client::started()
{
  root_.started();
}

void Client::stopped()
{
  root_.stopped();
}

void Client::clear()
{
  root_.categories.clear();
  root_.widgets.clear();
}

/** We rely on widgets to create categories */
void Client::category(const std::vector<std::string> &, const std::string &) {}

void Client::label(const ElementId & id, const std::string & txt)
{
  widget<Label>(id).data(txt);
}

void Client::array_label(const ElementId & id, const std::vector<std::string> & labels, const Eigen::VectorXd & data)
{
  widget<ArrayLabel>(id).data(labels, data);
}

void Client::button(const ElementId & id)
{
  widget<Button>(id);
}

void Client::checkbox(const ElementId & id, bool state)
{
  widget<Checkbox>(id).data(state);
}

void Client::point3d(const ElementId & id,
                     const ElementId & requestId,
                     bool ro,
                     const Eigen::Vector3d & pos,
                     const mc_rtc::gui::PointConfig & config)
{
  auto & point = widget<Point3D>(id, requestId);
  point.data(ro, pos, config);
}

void Client::robot(const ElementId & id,
                   const std::vector<std::string> & params,
                   const std::vector<std::vector<double>> & q)
{
  auto & robot = widget<Robot>(id, params);
  robot.data(q);
}

auto Client::getCategory(const std::vector<std::string> & category) -> Category &
{
  std::reference_wrapper<Category> out(root_);
  for(size_t i = 0; i < category.size(); ++i)
  {
    auto & cat = out.get();
    auto & next = category[i];
    auto it = std::find_if(cat.categories.begin(), cat.categories.end(), [&](auto & c) { return c->name == next; });
    if(it != cat.categories.end())
    {
      out = std::ref(*it->get());
    }
    else
    {
      out = *cat.categories.emplace_back(std::make_unique<Category>(next, cat.depth + 1));
    }
  }
  return out.get();
}
