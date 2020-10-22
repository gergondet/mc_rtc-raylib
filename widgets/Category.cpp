#include "Category.h"

void Category::update(Client & client, SceneState & state)
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

void Category::draw2D()
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

void Category::draw3D(Camera camera)
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

void Category::started()
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

void Category::stopped()
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
