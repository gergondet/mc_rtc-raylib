#pragma once

#include "Widget.h"

#include "../utils.h"

struct Polygon : public Widget
{

  Polygon(Client & client, const ElementId & id) : Widget(client, id) {}

  void data(const std::vector<std::vector<Eigen::Vector3d>> & points, const mc_rtc::gui::LineConfig & config)
  {
    if(points_ != points)
    {
      points_ = points;
    }
    config_ = config;
  }

  void draw3D(Camera) override
  {
    Color c = convert(config_.color);
    auto drawPoly = [this, &c](const std::vector<Eigen::Vector3d> & points) {
      for(size_t i = 0; i < points.size(); ++i)
      {
        const auto & p0 = points[i];
        const auto & p1 = i + 1 == points.size() ? points[0] : points[i + 1];
        DrawLine3DEx(convert(p0), convert(p1), c, config_.width, config_.style == mc_rtc::gui::LineStyle::Dotted);
      }
    };
    for(const auto & p : points_)
    {
      drawPoly(p);
    }
  }

private:
  std::vector<std::vector<Eigen::Vector3d>> points_;
  mc_rtc::gui::LineConfig config_;
};
