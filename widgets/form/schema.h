#pragma once

#include "widgets.h"

namespace form
{

struct ObjectForm : public Widget
{
  ObjectForm(const ::Widget & parent,
             const std::string & name,
             const std::map<std::string, mc_rtc::Configuration> & properties,
             const std::vector<std::string> & required);

  bool ready() override;

  void draw(bool show_header);

  void draw() override;

  void collect(mc_rtc::Configuration & out) override;

  std::optional<std::string> value(const std::string & name) const;

protected:
  std::vector<WidgetPtr> required_;
  std::vector<WidgetPtr> widgets_;
};

} // namespace form
