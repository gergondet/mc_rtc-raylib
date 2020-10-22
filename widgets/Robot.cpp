#include "../Client.h"

#include <mc_rbdyn/RobotLoader.h>

Client::Robot::Robot(const std::string & name, const std::vector<std::string> & p) : Widget(name)
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
    mc_rtc::log::error("RobotWidget {} cannot be displayed as the RobotModule cannot be loaded", name);
    return;
  }
  robots_ = mc_rbdyn::loadRobot(*rm);
  model_ = std::make_unique<RobotModel>(robots_->robot());
}

void Client::Robot::data(const std::vector<std::vector<double>> & q)
{
  if(model_)
  {
    robots_->robot().mbc().q = q;
    robots_->robot().forwardKinematics();
    model_->update(robots_->robot());
  }
}

void Client::Robot::draw3D(Camera camera)
{
  if(model_)
  {
    model_->draw(camera);
  }
}

