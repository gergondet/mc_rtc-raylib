#include "Point3D.h"

#include "../utils.h"

Point3D::Point3D(Client & client, const ElementId & id, const ElementId & requestId)
: TransformBase(client, id, requestId)
{
}

void Point3D::data(bool ro, const Eigen::Vector3d & pos, const mc_rtc::gui::PointConfig & config)
{
  TransformBase::data(ro, pos);
  config_ = config;
}

void Point3D::draw3D(Camera camera)
{
  TransformBase::draw3D(camera);
  DrawSphere(translation(pos_), static_cast<float>(config_.scale), convert(config_.color));
}
