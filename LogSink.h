#pragma once

#include <spdlog/sinks/base_sink.h>

#include <vector>

/** Simple log sink */
struct LogSink : public spdlog::sinks::base_sink<std::mutex>
{
  enum class Level
  {
    NONE,
    SUCCESS,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
  };

  struct Message
  {
    Level level;
    std::string msg;
  };

  inline void flush_() override {}

  inline void sink_it_(const spdlog::details::log_msg & msg) override
  {
    auto levelToLevel = [](const spdlog::level::level_enum & l) {
      switch(l)
      {
        case spdlog::level::info:
          return Level::INFO;
        case spdlog::level::warn:
          return Level::WARNING;
        case spdlog::level::err:
          return Level::ERROR;
        case spdlog::level::critical:
          return Level::CRITICAL;
        default:
          return Level::NONE;
      }
    };
    msgs_.push_back({levelToLevel(msg.level), {msg.payload.data(), msg.payload.size()}});
  }

  inline const std::vector<Message> & msgs() const
  {
    return msgs_;
  }

  inline void clear()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    msgs_.clear();
  }

  inline void addMessage(Level level, std::string && msg)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    msgs_.push_back({level, std::move(msg)});
  }

protected:
  std::vector<Message> msgs_;
};

using LogSinkPtr = std::shared_ptr<LogSink>;

struct SuccessSink : public LogSink
{
  inline SuccessSink(LogSinkPtr parent) : parent_(parent) {}

  inline void sink_it_(const spdlog::details::log_msg & msg) override
  {
    auto lvl = msg.level == spdlog::level::info ? Level::SUCCESS : Level::NONE;
    parent_->addMessage(lvl, {msg.payload.data(), msg.payload.size()});
  }

private:
  LogSinkPtr parent_;
};

using SuccessSinkPtr = std::shared_ptr<SuccessSink>;
