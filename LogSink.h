#pragma once

#include <spdlog/sinks/base_sink.h>

#include <vector>

/** Simple log sink */
struct LogSink : public spdlog::sinks::base_sink<std::mutex>
{
  struct Message
  {
    spdlog::level::level_enum level;
    std::string msg;
  };

  inline void flush_() override {}

  inline void sink_it_(const spdlog::details::log_msg & msg) override
  {
    msgs_.push_back({msg.level, {msg.payload.data(), msg.payload.size()}});
  }

  inline const std::vector<Message> & msgs() const
  {
    return msgs_;
  }

private:
  std::vector<Message> msgs_;
};
