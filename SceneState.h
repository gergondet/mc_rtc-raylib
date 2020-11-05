#pragma once

#include "raylib.h"

#include <functional>
#include <limits>

/** Represent the scene state */
struct SceneState
{
  /** 3D Camera used in the scene */
  Camera * camera;

  /** Reset the handler candidate for this loop */
  void startUpdate();

  /** Attempt to capture the mouse, the handler is decided after all objects
   * have been updated based on who's closest to the camera.
   *
   * The \param notify callback will be called if the \param source can take
   * hold of the mouse at the end of update.
   *
   * It should return true if the source want to hold onto the mouse for the
   * next frames */
  void attemptMouseCapture(void * source, float distance, std::function<bool()> notify);

  /** True if \param source holds the mouse at this time */
  bool hasMouse(void * source);

  /** Release the mouse, only has an effect if \param source currently holds the mouse */
  void releaseMouse(void * source);

  /** Notify the candidate that the mouse has been capture */
  void endUpdate();

private:
  /** Point to the current handler of mouse motions */
  void * mouseHandler_ = nullptr;
  struct Candidate
  {
    void * candidate = nullptr;
    float distance = std::numeric_limits<float>::infinity();
    std::function<bool()> notify;
  };
  Candidate candidate_;
};
