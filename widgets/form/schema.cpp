#include "schema.h"

namespace
{

template<typename T>
std::optional<T> get_default(const mc_rtc::Configuration & conf)
{
  if(!conf.has("default"))
  {
    return std::nullopt;
  }
  return conf("default", T{});
}

} // namespace

namespace form
{

ObjectForm::ObjectForm(const ::Widget & parent,
                       const std::string & name,
                       const std::map<std::string, mc_rtc::Configuration> & properties,
                       const std::vector<std::string> & required)
: form::Widget(parent, name)
{
  for(const auto & p : properties)
  {
    bool is_required = std::find(required.begin(), required.end(), p.first) != required.end();
    std::unique_ptr<form::Widget> widget;
    std::string nextName = fmt::format("{}##{}", p.first, name);
    if(p.second.has("enum"))
    {
      widget = std::make_unique<ComboInput>(parent, nextName, p.second("enum"), false);
    }
    else
    {
      std::string type = p.second("type", std::string(""));
      if(type == "boolean")
      {
        widget = std::make_unique<Checkbox>(parent, nextName, get_default<bool>(p.second));
      }
      else if(type == "integer")
      {
        if(p.first == "robotIndex")
        {
          widget = std::make_unique<DataComboInput>(parent, nextName, std::vector<std::string>{"robots"}, true);
        }
        else
        {
          widget = std::make_unique<IntegerInput>(parent, nextName, get_default<int>(p.second));
        }
      }
      else if(type == "number")
      {
        widget = std::make_unique<NumberInput>(parent, nextName, get_default<double>(p.second));
      }
      else if(type == "string")
      {
        if(p.first == "robot" || p.first == "r1" || p.first == "r2")
        {
          widget = std::make_unique<DataComboInput>(parent, nextName, std::vector<std::string>{"robots"}, false);
        }
        else if(p.first == "body")
        {
          widget = std::make_unique<DataComboInput>(
              parent, nextName, std::vector<std::string>{"bodies", fmt::format("$robot##{}", name)}, false);
        }
        else if(p.first == "surface" || p.first == "r1Surface" || p.first == "r2Surface")
        {
          std::string key = p.first == "surface" ? "$robot" : p.first == "r1Surface" ? "$r1" : "$r2";
          key = fmt::format("{}##{}", key, name);
          widget = std::make_unique<DataComboInput>(parent, nextName, std::vector<std::string>{"surfaces", key}, false);
        }
        else
        {
          widget = std::make_unique<StringInput>(parent, nextName, get_default<std::string>(p.second));
        }
      }
      else if(type == "array")
      {
        // FIXME
      }
      else if(type == "object")
      {
        widget = std::make_unique<ObjectForm>(parent, nextName, p.second("properties"),
                                              p.second("required", std::vector<std::string>{}));
      }
      else
      {
        mc_rtc::log::error("Cannot handle unknown type {} for property {} in {}", type, p.first, name);
      }
    }
    if(!widget)
    {
      mc_rtc::log::error("Failed to load a widget for property {} in {}", p.first, name);
      continue;
    }
    if(is_required)
    {
      required_.push_back(std::move(widget));
    }
    else
    {
      widgets_.push_back(std::move(widget));
    }
  }
}

bool ObjectForm::ready()
{
  return std::all_of(required_.begin(), required_.end(), [](auto && w) { return w->ready(); });
}

void ObjectForm::draw(bool show_header)
{
  if(!show_header || ImGui::CollapsingHeader(label(name_).c_str()))
  {
    if(show_header)
    {
      ImGui::Indent();
    }
    for(auto & w : required_)
    {
      w->draw();
      ImGui::Separator();
    }
    if(widgets_.size())
    {
      ImGui::Text("Optional fields");
      ImGui::Separator();
      ImGui::Indent();
      for(auto & w : widgets_)
      {
        w->draw();
        ImGui::Separator();
      }
      ImGui::Unindent();
    }
    if(show_header)
    {
      ImGui::Unindent();
    }
  }
}

void ObjectForm::collect(mc_rtc::Configuration & out)
{
  assert(ready());
  for(auto & w : required_)
  {
    w->collect(out);
  }
  for(auto & w : widgets_)
  {
    if(w->ready())
    {
      w->collect(out);
    }
  }
}

void ObjectForm::draw()
{
  draw(true);
}

std::optional<std::string> ObjectForm::value(const std::string & name) const
{
  auto value_ = [&name](const std::vector<WidgetPtr> & widgets) -> std::optional<std::string> {
    for(const auto & w : widgets)
    {
      if(w->fullName() == name)
      {
        return w->value();
      }
      else
      {
        auto obj = dynamic_cast<const ObjectForm *>(w.get());
        if(obj)
        {
          auto value = obj->value(name);
          if(value)
          {
            return value;
          }
        }
      }
    }
    return std::nullopt;
  };
  auto req = value_(required_);
  return req ? req : value_(widgets_);
}

} // namespace form
