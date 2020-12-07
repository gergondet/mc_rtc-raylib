#include "InteractiveMarker.h"

#include "utils.h"

#ifndef M_PI
#  define M_PI 3.14159265358979323846264338327950288
#endif

InteractiveMarker::MarkerPositionControl::MarkerPositionControl(Color color, Eigen::Vector3d offset)
: color(color), offset(offset)
{
}

InteractiveMarker::MarkerOrientationControl::MarkerOrientationControl(Model torus, Color c, Vector3 normal, Matrix ori)
: torus(torus), color(c), normal(normal), orientation(ori)
{
}

InteractiveMarker::InteractiveMarker(const sva::PTransformd & pose, ControlAxis mask) : pose_(pose)
{
  auto check = [&](ControlAxis value) { return (mask & value) == value; };
  if(check(ControlAxis::TX))
  {
    posControls_.emplace_back(RED, Eigen::Vector3d::UnitX());
    posControls_.emplace_back(RED, -Eigen::Vector3d::UnitX());
  }
  if(check(ControlAxis::TY))
  {
    posControls_.emplace_back(GREEN, Eigen::Vector3d::UnitY());
    posControls_.emplace_back(GREEN, -Eigen::Vector3d::UnitY());
  }
  if(check(ControlAxis::TZ))
  {
    posControls_.emplace_back(BLUE, Eigen::Vector3d::UnitZ());
    posControls_.emplace_back(BLUE, -Eigen::Vector3d::UnitZ());
  }
  if(check(ControlAxis::RX))
  {
    oriControls_.emplace_back(LoadModelFromMesh(GenMeshTorus(0.1f, 0.3f, 8, 32)), RED, Vector3{1, 0, 0},
                              MatrixRotateY(M_PI / 2));
  }
  if(check(ControlAxis::RY))
  {
    oriControls_.emplace_back(LoadModelFromMesh(GenMeshTorus(0.1f, 0.3f, 8, 32)), GREEN, Vector3{0, 1, 0},
                              MatrixRotateX(M_PI / 2));
  }
  if(check(ControlAxis::RZ))
  {
    oriControls_.emplace_back(LoadModelFromMesh(GenMeshTorus(0.1f, 0.3f, 8, 32)), BLUE, Vector3{0, 0, 1});
  }
  this->pose(pose_);
}

void InteractiveMarker::pose(const sva::PTransformd & pose)
{
  pose_ = pose;
  double sizeMul = 0.15;
  for(auto & c : posControls_)
  {
    c.pose = translation(sva::PTransformd(Eigen::Vector3d(sizeMul * c.offset)) * pose_);
    c.bbox.min = Vector3SubtractValue(c.pose, size / 2);
    c.bbox.max = Vector3AddValue(c.pose, size / 2);
  }
  for(auto & c : oriControls_)
  {
    c.torus.transform = MatrixMultiply(c.orientation, convert(pose_));
  }
}

void InteractiveMarker::update(SceneState & state)
{
  auto & camera = *state.camera;
  auto ray = GetMouseRay(GetMousePosition(), camera);
  bool posCandidate = false;
  for(auto & c : posControls_)
  {
    if(state.hasMouse(&c))
    {
      if(IsMouseButtonReleased(0))
      {
        active_ = false;
        c.active = false;
        state.releaseMouse(&c);
      }
      else
      {
        Vector2 now = GetMousePosition();
        Vector2 move = Vector2Subtract(now, c.start);
        float dirL = Vector2LengthSqr(c.dir);
        if(dirL == 0.0f)
        {
          return;
        }
        float mag = Vector2DotProduct(move, c.dir) / dirL;
        c.start = now;
        sva::PTransformd offset{Eigen::Vector3d(mag * c.offset)};
        pose(offset * pose_);
      }
    }
    else if(CheckCollisionRayBox(ray, c.bbox))
    {
      c.hover = false;
      float distance = Vector3Length(Vector3Subtract(c.pose, ray.position));
      posCandidate = true;
      state.attemptMouseCapture(&c, distance, [&c, &camera, this]() {
        c.hover = true;
        if(IsMouseButtonPressed(0))
        {
          active_ = true;
          c.active = true;
          c.start = GetMousePosition();
          Vector2 start = GetWorldToScreen(translation(pose_), camera);
          Vector2 end = GetWorldToScreen(translation(sva::PTransformd(c.offset) * pose_), camera);
          c.dir = Vector2Subtract(end, start);
          return true;
        }
        return false;
      });
    }
    else
    {
      c.hover = false;
    }
  }
  for(auto & c : oriControls_)
  {
    if(state.hasMouse(&c))
    {
      if(IsMouseButtonReleased(0))
      {
        active_ = false;
        c.active = false;
        state.releaseMouse(&c);
      }
      else
      {
        Vector3 point =
            Vector3Normalize(Vector3Subtract(intersection(ray, c.pZ, translation(pose_)), translation(pose_)));
        float x = Vector3DotProduct(point, c.pX);
        float y = Vector3DotProduct(point, c.pY);
        float theta = atan2(y, x);
        Eigen::Vector3d normal{c.normal.x, c.normal.y, c.normal.z};
        sva::PTransformd offset(Eigen::Matrix3d(Eigen::AngleAxisd(-theta, normal)));
        pose(offset * pose_);
        c.pX = point;
        c.pY = Vector3Normalize(Vector3CrossProduct(c.pZ, c.pX));
      }
    }
    else
    {
      c.hover = false;
      if(posCandidate)
      {
        continue;
      }
      auto hit = GetCollisionRayModel(ray, c.torus);
      if(!hit.hit)
      {
        continue;
      }
      state.attemptMouseCapture(&c, hit.distance, [&c, ray, this]() {
        c.hover = true;
        if(IsMouseButtonPressed(0))
        {
          active_ = true;
          c.active = true;
          c.pZ = Vector3Transform(c.normal, convert(pose_.rotation()));
          Vector3 point = intersection(ray, c.pZ, translation(pose_));
          c.pX = Vector3Normalize(Vector3Subtract(point, translation(pose_)));
          c.pY = Vector3Normalize(Vector3CrossProduct(c.pZ, c.pX));
          return true;
        }
        return false;
      });
    }
  }
}

void InteractiveMarker::draw()
{
  auto transparent = [](Color c) { return Color{c.r, c.g, c.b, 128}; };
  auto color = [transparent](auto & c) {
    if(c.hover || c.active)
    {
      return c.color;
    }
    return transparent(c.color);
  };
  for(auto & c : posControls_)
  {
    DrawCube(c.pose, size, size, size, color(c));
    if(c.hover || c.active)
    {
      DrawCubeWires(c.pose, size, size, size, c.active ? WHITE : GRAY);
    }
  }
  for(auto & c : oriControls_)
  {
    DrawModel(c.torus, Vector3Zero(), 1.0f, color(c));
    if(c.active)
    {
      DrawModelWires(c.torus, Vector3Zero(), 1.0f, WHITE);
    }
  }
}
