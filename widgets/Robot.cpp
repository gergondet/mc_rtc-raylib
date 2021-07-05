#include "Robot.h"

#include <mc_rbdyn/RobotLoader.h>

#include <mc_rtc/version.h>

Robot::Robot(Client & client, const ElementId & id) : Widget(client, id)
{
}

void Robot::reset(const std::vector<std::string> & p)
{
  params_ = p;
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

void Robot::data(const std::vector<std::string> & params, const std::vector<std::vector<double>> & q, const sva::PTransformd & posW)
{
  if(params != params_)
  {
    reset(params);
  }
  if(robots_)
  {
    auto & robot = robots_->robot();
#if MC_RTC_VERSION_MAJOR < 2
    robot.mbc().q = q;
#else
    robot.q()->set(rbd::paramToVector(robot.mb(), q));
#endif
    robot.posW(posW);
  }
  if(display_ && model_)
  {
    model_->update(robots_->robot());
  }
  if(displayCollision_)
  {
    if(!collisionModel_)
    {
      collisionModel_ = std::make_unique<RobotModel>(robots_->robot(), true);
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
