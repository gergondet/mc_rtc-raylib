#pragma once

#include "Widget.h"

#include "../InteractiveMarker.h"
#include "../utils.h"

struct Arrow : public Widget
{
  Arrow(Client & client, const ElementId & id, const ElementId & reqId) : Widget(client, id), requestId_(reqId) {}

  void data(const Eigen::Vector3d & start,
            const Eigen::Vector3d & end,
            const mc_rtc::gui::ArrowConfig & config,
            bool ro)
  {
    if(ro && startMarker_)
    {
      startMarker_.reset(nullptr);
      endMarker_.reset(nullptr);
    }
    if(!ro && !startMarker_)
    {
      startMarker_ = std::make_unique<InteractiveMarker>(start, ControlAxis::TRANSLATION);
      endMarker_ = std::make_unique<InteractiveMarker>(end, ControlAxis::TRANSLATION);
    }
    start_ = start;
    end_ = end;
    if(startMarker_ && !startMarker_->active())
    {
      startMarker_->pose(start);
    }
    if(endMarker_ && !endMarker_->active())
    {
      endMarker_->pose(end);
    }
    config_ = config;
  }

  void update(SceneState & state)
  {
    bool changed = false;
    if(startMarker_)
    {
      startMarker_->update(state);
      if(startMarker_->active())
      {
        start_ = startMarker_->pose().translation();
        changed = true;
      }
    }
    if(endMarker_)
    {
      endMarker_->update(state);
      if(endMarker_->active())
      {
        end_ = endMarker_->pose().translation();
        changed = true;
      }
    }
    if(changed)
    {
      Eigen::Vector6d data;
      data << start_, end_;
      client.send_request(requestId_, data);
    }
  }

  void draw3D(Camera)
  {
    DrawArrow(convert(start_), convert(end_), config_.shaft_diam, config_.head_diam, config_.head_len,
              convert(config_.color));
    if(startMarker_)
    {
      startMarker_->draw();
      endMarker_->draw();
    }
  }

private:
  ElementId requestId_;
  Eigen::Vector3d start_;
  Eigen::Vector3d end_;
  mc_rtc::gui::ArrowConfig config_;
  InteractiveMarkerPtr startMarker_;
  InteractiveMarkerPtr endMarker_;
};
