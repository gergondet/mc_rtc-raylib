#include "Robot.h"

#include <mc_rbdyn/RobotLoader.h>

Robot::Robot(const ElementId & id, const std::vector<std::string> & p) : Widget(id)
{
  mc_rbdyn::RobotModulePtr rm{nullptr};
  if(p.size() == 1)
  {
    rm = mc_rbdyn::RobotLoader::get_robot_module(p[0]);
  }
  if(p.size() == 2)
  {
    rm = mc_rbdyn::RobotLoader::get_robot_module(p[0], p[1]);
  }
  if(p.size() == 3)
  {
    rm = mc_rbdyn::RobotLoader::get_robot_module(p[0], p[1], p[2]);
  }
  if(p.size() > 3)
  {
    mc_rtc::log::warning("Too many parameters provided to load the robot, complain to the developpers of this package");
  }
  if(!rm)
  {
    mc_rtc::log::error("RobotWidget {} cannot be displayed as the RobotModule cannot be loaded", id.name);
    return;
  }
  robots_ = mc_rbdyn::loadRobot(*rm);
  model_ = std::make_unique<RobotModel>(robots_->robot());
}

void Robot::data(const std::vector<std::vector<double>> & q)
{
  if(robots_)
  {
    robots_->robot().mbc().q = q;
    robots_->robot().forwardKinematics();
  }
  if(display_ && model_)
  {
    model_->update(robots_->robot());
  }
  if(displayCollision_)
  {
    if(!collisionModel_)
    {
      collisionModel_  = std::make_unique<RobotModel>(robots_->robot(), true);
    }
    collisionModel_->update(robots_->robot());
  }
}

void Robot::draw2D()
{
  ImGui::Checkbox(label(fmt::format("Display {}", id.name)).c_str(), &display_);
  ImGui::Checkbox(label(fmt::format("Display {} collision model", id.name)).c_str(), &displayCollision_);
}

void Robot::draw3D(Camera camera)
{
  if(model_ && display_)
  {
    model_->draw(camera);
  }
  if(collisionModel_ && displayCollision_)
  {
    collisionModel_->draw(camera);
  }
}

