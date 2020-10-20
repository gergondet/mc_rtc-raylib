#pragma once

#include <SpaceVecAlg/SpaceVecAlg>
#include <SpaceVecAlg/Conversions.h>

#include "raylib.h"
#include "raymath.h"

// Convert a PTransform into a Matrix
Matrix convert(const sva::PTransformd & pt);

inline Matrix convert(const Eigen::Matrix3d & m)
{
  return convert(sva::PTransformd{m});
}

// Extract the translation part of a PTransform only
Vector3 translation(const sva::PTransformd & pt);

// Given a ray and a plane (normal, point) returns the intersection point
Vector3 intersection(Ray ray, Vector3 normal, Vector3 point);

// Same implementation as raylib's DrawGrid but draw on the XY plane instead of XZ
void DrawGridXY(int slices, float spacing);
