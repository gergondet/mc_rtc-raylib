#include "Client.h"

#include "widgets/ArrayInput.h"
#include "widgets/ArrayLabel.h"
#include "widgets/Button.h"
#include "widgets/Checkbox.h"
#include "widgets/ComboInput.h"
#include "widgets/DataComboInput.h"
#include "widgets/IntegerInput.h"
#include "widgets/Label.h"
#include "widgets/NumberInput.h"
#include "widgets/NumberSlider.h"
#include "widgets/Point3D.h"
#include "widgets/Polygon.h"
#include "widgets/Robot.h"
#include "widgets/StringInput.h"
#include "widgets/Trajectory.h"

void Client::update(SceneState & state)
{
  root_.update(state);
  run(buffer_, t_last_);
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

void Client::string_input(const ElementId & id, const std::string & data)
{
  widget<StringInput>(id).data(data);
}

void Client::integer_input(const ElementId & id, int data)
{
  widget<IntegerInput>(id).data(data);
}

void Client::number_input(const ElementId & id, double data)
{
  widget<NumberInput>(id).data(data);
}

void Client::number_slider(const ElementId & id, double data, double min, double max)
{
  widget<NumberSlider>(id).data(data, min, max);
}

void Client::array_input(const ElementId & id, const std::vector<std::string> & labels, const Eigen::VectorXd & data)
{
  widget<ArrayInput>(id).data(labels, data);
}

void Client::combo_input(const ElementId & id, const std::vector<std::string> & values, const std::string & data)
{
  widget<ComboInput>(id).data(values, data);
}

void Client::data_combo_input(const ElementId & id, const std::vector<std::string> & values, const std::string & data)
{
  widget<DataComboInput>(id).data(values, data);
}

void Client::point3d(const ElementId & id,
                     const ElementId & requestId,
                     bool ro,
                     const Eigen::Vector3d & pos,
                     const mc_rtc::gui::PointConfig & config)
{
  widget<Point3D>(id, requestId).data(ro, pos, config);
}

void Client::trajectory(const ElementId & id,
                        const std::vector<Eigen::Vector3d> & points,
                        const mc_rtc::gui::LineConfig & config)
{
  widget<Trajectory<Eigen::Vector3d>>(id).data(points, config);
}

void Client::trajectory(const ElementId & id,
                        const std::vector<sva::PTransformd> & points,
                        const mc_rtc::gui::LineConfig & config)
{
  widget<Trajectory<sva::PTransformd>>(id).data(points, config);
}

void Client::trajectory(const ElementId & id, const Eigen::Vector3d & point, const mc_rtc::gui::LineConfig & config)
{
  widget<Trajectory<Eigen::Vector3d>>(id).data(point, config);
}

void Client::trajectory(const ElementId & id, const sva::PTransformd & point, const mc_rtc::gui::LineConfig & config)
{
  widget<Trajectory<sva::PTransformd>>(id).data(point, config);
}

void Client::polygon(const ElementId & id,
                     const std::vector<std::vector<Eigen::Vector3d>> & points,
                     const mc_rtc::gui::Color & color)
{
  widget<Polygon>(id).data(points, color);
}

void Client::robot(const ElementId & id,
                   const std::vector<std::string> & params,
                   const std::vector<std::vector<double>> & q)
{
  widget<Robot>(id, params).data(q);
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
