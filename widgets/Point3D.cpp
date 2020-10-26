#include "Point3D.h"

#include "../utils.h"

Point3D::Point3D(Client & client, const ElementId & id, const ElementId & requestId)
: Widget(client, id), requestId_(requestId)
{
}

void Point3D::data(bool ro, const Eigen::Vector3d & pos, const mc_rtc::gui::PointConfig & config)
{
  if(!ro && !marker_)
  {
    marker_ = std::make_unique<InteractiveMarker>(pos, ControlAxis::TRANSLATION);
  }
  if(ro && marker_)
  {
    marker_.reset(nullptr);
  }
  if(marker_ && !marker_->active())
  {
    marker_->pose(pos);
  }
  pos_ = convert(pos);
  config_ = config;
}

void Point3D::update(SceneState & state)
{
  if(marker_)
  {
    marker_->update(state);
    if(marker_->active())
    {
      client.send_request(requestId_, marker_->pose().translation());
    }
  }
}

void Point3D::draw3D(Camera)
{
  if(marker_)
  {
    marker_->draw();
  }
  DrawSphere(pos_, static_cast<float>(config_.scale), convert(config_.color));
}
