#include "Client.h"

void Client::update(SceneState & state)
{
  run(buffer_, t_last_);
  root_.update(*this, state);
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

void Client::point3d(const ElementId & id,
                     const ElementId & requestId,
                     bool ro,
                     const Eigen::Vector3d & pos,
                     const mc_rtc::gui::PointConfig & config)
{
  auto & point = widget<Point3D>(id, requestId);
  point.data(ro, pos, config);
}

void Client::robot(const ElementId & id,
                   const std::vector<std::string> & params,
                   const std::vector<std::vector<double>> & q)
{
  auto & robot = widget<Robot>(id, params);
  robot.data(q);
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

void Client::Category::update(Client & client, SceneState & state)
{
  for(auto & w : widgets)
  {
    w->update(client, state);
  }
  for(auto & cat : categories)
  {
    cat->update(client, state);
  }
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
  /** Remove empty categories */
  {
    auto it =
        std::remove_if(categories.begin(), categories.end(), [](const auto & c) { return c->widgets.size() == 0 && c->categories.size() == 0; });
    categories.erase(it, categories.end());
  }
  /** Remove widgets that have not been seen */
  {
    auto it = std::remove_if(widgets.begin(), widgets.end(), [](const auto & w) { return !w->seen; });
    widgets.erase(it, widgets.end());
  }
}

