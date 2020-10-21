#include "Camera.h"

#include "raymath.h"

#include <stdio.h>

void OrbitCamera::update()
{
  if(IsMouseButtonReleased(0))
  {
    rotate = false;
  }
  if(IsMouseButtonReleased(2))
  {
    pan = false;
  }
  if(pan || rotate)
  {
    Vector3 direction = Vector3Normalize(Vector3Subtract(position, target));
    Vector3 right = Vector3CrossProduct(up, direction);
    Vector3 upAx = Vector3CrossProduct(direction, right);
    if(pan)
    {
      float scale = 0.01;
      Vector3 dright = Vector3Scale(right, -scale*(GetMousePosition().x - start.x));
      Vector3 dup = Vector3Scale(upAx, scale*(GetMousePosition().y - start.y));
      Vector3 diff = Vector3Add(dright, dup);
      start = GetMousePosition();
      position = Vector3Add(position, diff);
      target = Vector3Add(target, diff);
    }
    if(rotate)
    {
      Vector3 s = Vector3Subtract(position, target);
      float r = Vector3Length(s);
      float phi = atan2(s.y, s.x);
      float theta = acos(s.z / r);
      float scale = -0.01;
      float dphi = scale * (GetMousePosition().x - start.x);
      float dtheta = scale * (GetMousePosition().y - start.y);
      Vector3 ns = { r * sin(theta + dtheta) * cos(phi + dphi),
                     r * sin(theta + dtheta) * sin(phi + dphi),
                     r * cos(theta + dtheta) };
      position = Vector3Add(target, ns);
      start = GetMousePosition();
    }
  }
  else if(GetMouseWheelMove() != 0)
  {
    Vector3 direction = Vector3Normalize(Vector3Subtract(position, target));
    int move = GetMouseWheelMove();
    float distance = Vector3Length(Vector3Subtract(target, position));
    if(move > 0)
    {
      position = Vector3Add(position, Vector3Scale(direction, -0.1 * distance));
    }
    else
    {
      position = Vector3Add(position, Vector3Scale(direction, 0.1 * distance));
    }
  }
  if(IsMouseButtonPressed(0))
  {
    rotate = true;
    start = GetMousePosition();
  }
  if(IsMouseButtonPressed(2))
  {
    pan = true;
    start = GetMousePosition();
  }
}
