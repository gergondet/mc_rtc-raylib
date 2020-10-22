#include "../Client.h"

Client::Point3D::Point3D(const std::string & name, const ElementId & requestId) : Widget(name), requestId_(requestId) {}

void Client::Point3D::data(bool ro, const Eigen::Vector3d & pos, const mc_rtc::gui::PointConfig & config)
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
}

void Client::Point3D::update(Client & client, SceneState & state)
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

void Client::Point3D::draw3D(Camera)
{
  if(marker_)
  {
    marker_->draw();
  }
}
