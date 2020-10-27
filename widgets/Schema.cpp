#include "Schema.h"

#include <mc_rtc/config.h>

namespace
{

std::string removeFakeDir(const std::string & in)
{
  std::string_view fakeDir = "/../";
  if(in.size() >= fakeDir.size() && in.substr(0, fakeDir.size()) == fakeDir)
  {
    return in.substr(fakeDir.size());
  }
  if(in.size() >= fakeDir.size() && in.substr(0, fakeDir.size() - 1) == fakeDir.substr(1))
  {
    return in.substr(fakeDir.size() - 1);
  }
  return in;
}

void resolveRef(const bfs::path & path,
                mc_rtc::Configuration conf,
                const std::function<mc_rtc::Configuration(const bfs::path &)> & loadFn)
{
  if(conf.size())
  {
    for(size_t i = 0; i < conf.size(); ++i)
    {
      resolveRef(path, conf[i], loadFn);
    }
  }
  else
  {
    auto keys = conf.keys();
    for(const auto & k : keys)
    {
      if(k == "$ref")
      {
        auto ref = loadFn(bfs::canonical(path.parent_path() / removeFakeDir(conf(k)).c_str()));
        conf.load(ref);
        conf.remove("$ref");
      }
      else
      {
        resolveRef(path, conf(k), loadFn);
      }
    }
  }
}

void resolveAllOf(mc_rtc::Configuration conf)
{
  if(conf.size())
  {
    for(size_t i = 0; i < conf.size(); ++i)
    {
      resolveAllOf(conf[i]);
    }
  }
  else
  {
    auto keys = conf.keys();
    for(const auto & k : keys)
    {
      if(k == "allOf")
      {
        std::vector<mc_rtc::Configuration> allOf = conf("allOf");
        for(auto & c : allOf)
        {
          resolveAllOf(c);
          conf.load(c);
        }
        conf.remove("allOf");
      }
      else
      {
        resolveAllOf(conf(k));
      }
    }
  }
}

} // namespace

Schema::Schema(Client & client, const ElementId & id) : Widget(client, id) {}

void Schema::data(const std::string & schema)
{
  if(schema == schema_)
  {
    return;
  }
  schema_ = schema;
  bfs::path all_schemas = bfs::path(mc_rtc::INSTALL_PREFIX) / "share" / "doc" / "mc_rtc" / "json" / "schemas";
  bfs::path schema_dir = all_schemas / schema_.c_str();
  if(!bfs::exists(schema_dir) || !bfs::is_directory(schema_dir))
  {
    mc_rtc::log::error("Cannot load schema from non existing directory: {}", schema_dir.string());
    return;
  }
  bfs::directory_iterator dit(schema_dir), endit;
  std::vector<bfs::path> schemas;
  std::copy(dit, endit, std::back_inserter(schemas));
  for(const auto & s : schemas)
  {
    auto & schema = loadSchema(s);
    schemas_[schema("title")] = schema;
    static bool show = true;
    if(show)
    {
      mc_rtc::log::info("SCHEMA: {}", s.string());
      mc_rtc::log::info("{}", schema.dump(true, true));
    }
  }
}

void Schema::draw2D() {}

mc_rtc::Configuration & Schema::loadSchema(const bfs::path & path)
{
  if(bfs::canonical(path) != path)
  {
    return loadSchema(bfs::canonical(path));
  }
  if(all_schemas_.count(path.string()))
  {
    return all_schemas_[path.string()];
  }
  if(!bfs::exists(path))
  {
    mc_rtc::log::error_and_throw<std::runtime_error>("No schema can be loaded from {}", path.string());
  }
  auto & schema = all_schemas_[path.string()];
  schema.load(path.string());
  resolveRef(path, schema, [this](const bfs::path & p) { return loadSchema(p); });
  resolveAllOf(schema);
  return schema;
}
