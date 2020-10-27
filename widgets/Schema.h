#pragma once

#include "Widget.h"

#include <boost/filesystem.hpp>
namespace bfs = boost::filesystem;

struct Schema : public Widget
{
  Schema(Client & client, const ElementId & id);

  ~Schema() override = default;

  void data(const std::string & schema);

  void draw2D() override;

private:
  mc_rtc::Configuration & loadSchema(const bfs::path & path);
  /** Schema directory used by this widget */
  std::string schema_;
  /** All schemas that should be presented by the user, indexed by title */
  std::map<std::string, mc_rtc::Configuration> schemas_;
  /** All schemas indexed by disk location */
  std::unordered_map<std::string, mc_rtc::Configuration> all_schemas_;
};
