#pragma once

#include <SpaceVecAlg/Conversions.h>
#include <SpaceVecAlg/SpaceVecAlg>

#include <mc_rtc/gui/types.h>

#include "raylib.h"
#include "raymath.h"

// Convert a PTransform into a Matrix
Matrix convert(const sva::PTransformd & pt);

inline Matrix convert(const Eigen::Matrix3d & m)
{
  return convert(sva::PTransformd{m});
}

inline Vector3 convert(const Eigen::Vector3d & v)
{
  return {static_cast<float>(v.x()), static_cast<float>(v.y()), static_cast<float>(v.z())};
}

inline Color convert(const mc_rtc::gui::Color & color)
{
  auto f = [](double d) { return static_cast<float>(d); };
  return ColorFromNormalized({f(color.r), f(color.g), f(color.b), f(color.a)});
}

// Extract the translation part of a PTransform only
Vector3 translation(const sva::PTransformd & pt);

inline Vector3 translation(const Eigen::Vector3d & v)
{
  return convert(v);
}

// Given a ray and a plane (normal, point) returns the intersection point
Vector3 intersection(Ray ray, Vector3 normal, Vector3 point);

// Same implementation as raylib's DrawGrid but draw on the XY plane instead of XZ
void DrawGridXY(int slices, float spacing);

// Draw a cylinder with the given normal direction
// Same implementation as raylib's DrawCylinder except we rotate the camera
void DrawCylinderEx(Vector3 position,
                    Vector3 normal,
                    float radiusTop,
                    float radiusBottom,
                    float height,
                    int sides,
                    Color color);

// Draw a 3D arrow
void DrawArrow(Vector3 p0, Vector3 p1, float shaft_diam, float head_diam, float head_len, Color color);

void DrawFrame(const sva::PTransformd & pose);
