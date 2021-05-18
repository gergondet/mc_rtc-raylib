#include "Client.h"

#include "widgets/ArrayInput.h"
#include "widgets/ArrayLabel.h"
#include "widgets/Arrow.h"
#include "widgets/Button.h"
#include "widgets/Checkbox.h"
#include "widgets/ComboInput.h"
#include "widgets/DataComboInput.h"
#include "widgets/Force.h"
#include "widgets/Form.h"
#include "widgets/IntegerInput.h"
#include "widgets/Label.h"
#include "widgets/NumberInput.h"
#include "widgets/NumberSlider.h"
#include "widgets/Point3D.h"
#include "widgets/Polygon.h"
#include "widgets/Robot.h"
#include "widgets/Rotation.h"
#include "widgets/Schema.h"
#include "widgets/StringInput.h"
#include "widgets/Table.h"
#include "widgets/Trajectory.h"
#include "widgets/Transform.h"
#include "widgets/XYTheta.h"

void Client::register_log_sink()
{
  if(!sink_)
  {
    sink_ = std::make_shared<LogSink>();
    successSink_ = std::make_shared<SuccessSink>(sink_);
  }
  mc_rtc::log::details::info().sinks().push_back(sink_);
  mc_rtc::log::details::success().sinks().push_back(successSink_);
  mc_rtc::log::details::cerr().sinks().push_back(sink_);
}

void Client::update(SceneState & state)
{
  root_.update(state);
  run(buffer_, t_last_);
}

void Client::draw2D()
{
  auto left_margin = 15;
  auto top_margin = 50;
  auto bottom_margin = 50;
  auto width = GetScreenWidth() - left_margin;
  auto height = GetScreenHeight() - top_margin - bottom_margin;
  if(!root_.empty())
  {
    ImGui::SetNextWindowPos(ImVec2(left_margin, top_margin), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(0.4 * width, 0.7 * height), ImGuiCond_FirstUseEver);
    ImGui::Begin("mc_rtc");
    root_.draw2D();
    ImGui::End();
  }
  if(sink_)
  {
    auto w_height = 0.25 * height;
    ImGui::SetNextWindowPos(ImVec2(left_margin, height + top_margin - w_height), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(0.4 * width, w_height), ImGuiCond_FirstUseEver);
    ImGui::Begin("Console");
    for(const auto & m : sink_->msgs())
    {
      switch(m.level)
      {
        case LogSink::Level::SUCCESS:
          ImGui::TextColored({50.0 / 255.0, 205.0 / 255.0, 50.0 / 255.0, 1}, "[success]");
          ImGui::SameLine();
          break;
        case LogSink::Level::INFO:
          ImGui::TextColored({0, 0, 1, 1}, "[info]");
          ImGui::SameLine();
          break;
        case LogSink::Level::WARNING:
          ImGui::TextColored({1, 0.5, 0, 1}, "[warning]");
          ImGui::SameLine();
          break;
        case LogSink::Level::ERROR:
          ImGui::TextColored({1, 0, 0, 1}, "[error]");
          ImGui::SameLine();
          break;
        case LogSink::Level::CRITICAL:
          ImGui::TextColored({1, 0, 0, 1}, "[CRITICAL]");
          ImGui::SameLine();
          break;
        default:
          break;
      }
      ImGui::Text("%s", m.msg.c_str());
    }
    ImGui::SetScrollHereY(1.0f);
    ImGui::End();
  }
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

void Client::clearConsole()
{
  if(sink_)
  {
    sink_->clear();
  }
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

void Client::force(const ElementId & id,
                   const ElementId & requestId,
                   const sva::ForceVecd & force,
                   const sva::PTransformd & pos,
                   const mc_rtc::gui::ForceConfig & forceConfig,
                   bool /* ro */)
{
  widget<Force>(id, requestId).data(force, pos, forceConfig);
}

void Client::arrow(const ElementId & id,
                   const ElementId & requestId,
                   const Eigen::Vector3d & start,
                   const Eigen::Vector3d & end,
                   const mc_rtc::gui::ArrowConfig & config,
                   bool ro)
{
  widget<Arrow>(id, requestId).data(start, end, config, ro);
}

void Client::rotation(const ElementId & id, const ElementId & requestId, bool ro, const sva::PTransformd & pos)
{
  widget<Rotation>(id, requestId).data(ro, pos);
}

void Client::transform(const ElementId & id, const ElementId & requestId, bool ro, const sva::PTransformd & pos)
{
  widget<TransformWidget>(id, requestId).data(ro, pos);
}

void Client::xytheta(const ElementId & id,
                     const ElementId & requestId,
                     bool ro,
                     const Eigen::Vector3d & xytheta,
                     double altitude)
{
  widget<XYTheta>(id, requestId).data(ro, xytheta, altitude);
}

void Client::table_start(const ElementId & id, const std::vector<std::string> & header)
{
  widget<Table>(id).start(header);
}

void Client::table_row(const ElementId & id, const std::vector<std::string> & data)
{
  widget<Table>(id).row(data);
}

void Client::table_end(const ElementId & id)
{
  widget<Table>(id).end();
}

void Client::form(const ElementId & id)
{
  widget<Form>(id);
}

void Client::form_checkbox(const ElementId & id, const std::string & name, bool required, bool default_)
{
  widget<Form>(id).widget<form::Checkbox>(name, required, default_);
}

void Client::form_integer_input(const ElementId & id, const std::string & name, bool required, int default_)
{
  widget<Form>(id).widget<form::IntegerInput>(name, required, default_);
}

void Client::form_number_input(const ElementId & id, const std::string & name, bool required, double default_)
{
  widget<Form>(id).widget<form::NumberInput>(name, required, default_);
}

void Client::form_string_input(const ElementId & id,
                               const std::string & name,
                               bool required,
                               const std::string & default_)
{
  widget<Form>(id).widget<form::StringInput>(name, required, default_);
}

void Client::form_array_input(const ElementId & id,
                              const std::string & name,
                              bool required,
                              const Eigen::VectorXd & default_,
                              bool fixed_size)
{
  widget<Form>(id).widget<form::ArrayInput>(name, required, default_, fixed_size);
}

void Client::form_combo_input(const ElementId & id,
                              const std::string & name,
                              bool required,
                              const std::vector<std::string> & values,
                              bool send_index)
{
  widget<Form>(id).widget<form::ComboInput>(name, required, values, send_index);
}

void Client::form_data_combo_input(const ElementId & id,
                                   const std::string & name,
                                   bool required,
                                   const std::vector<std::string> & ref,
                                   bool send_index)
{
  widget<Form>(id).widget<form::DataComboInput>(name, required, ref, send_index);
}

void Client::robot(const ElementId & id,
                   const std::vector<std::string> & params,
                   const std::vector<std::vector<double>> & q,
                   const sva::PTransformd & posW)
{
  widget<Robot>(id).data(params, q, posW);
}

void Client::schema(const ElementId & id, const std::string & schema)
{
  widget<Schema>(id).data(schema);
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
