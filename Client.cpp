#include "Client.h"

#include <mc_rbdyn/RobotLoader.h>

void Client::update(SceneState &)
{
  run(buffer_, t_last_);
}

void Client::draw2D()
{
  root_.draw2D();
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

void Client::robot(const ElementId & id,
                   const std::vector<std::string> & params,
                   const std::vector<std::vector<double>> & q)
{
  auto & robot = widget<Robot>(id, params);
  robot.update(q);
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

void Client::Category::draw2D()
{
  for(auto & w : widgets)
  {
    w->draw2D();
  }
  for(auto & cat : categories)
  {
    cat->draw2D();
  }
}

void Client::Category::draw3D(Camera camera)
{
  for(auto & w : widgets)
  {
    w->draw3D(camera);
  }
  for(auto & cat : categories)
  {
    cat->draw3D(camera);
  }
}

void Client::Category::started()
{
  for(auto & w : widgets)
  {
    w->seen = false;
  }
  for(auto & cat : categories)
  {
    cat->started();
  }
}

void Client::Category::stopped()
{
  /** Clean up categories first */
  for(auto & cat : categories)
  {
    cat->stopped();
  }
  /** Remove categories that have no active widgets */
  {
    auto it =
        std::remove_if(categories.begin(), categories.end(), [](const auto & c) { return c->widgets.size() == 0; });
    categories.erase(it, categories.end());
  }
  /** Remove widgets that have not been seen */
  {
    auto it = std::remove_if(widgets.begin(), widgets.end(), [](const auto & w) { return !w->seen; });
    widgets.erase(it, widgets.end());
  }
}

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

void Client::Robot::update(const std::vector<std::vector<double>> & q)
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
