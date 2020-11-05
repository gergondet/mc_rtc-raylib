#include "SceneState.h"

void SceneState::startUpdate()
{
  candidate_ = {};
}

void SceneState::attemptMouseCapture(void * source, float distance, std::function<bool()> notify)
{
  if(candidate_.candidate == nullptr || distance < candidate_.distance)
  {
    candidate_.candidate = source;
    candidate_.distance = distance;
    candidate_.notify = notify;
  }
}

bool SceneState::hasMouse(void * source)
{
  return mouseHandler_ == source;
}

void SceneState::releaseMouse(void * source)
{
  if(mouseHandler_ == source)
  {
    mouseHandler_ = nullptr;
  }
}

void SceneState::endUpdate()
{
  if(mouseHandler_ != nullptr)
  {
    return;
  }
  if(candidate_.candidate)
  {
    if(candidate_.notify())
    {
      mouseHandler_ = candidate_.candidate;
    }
  }
}
