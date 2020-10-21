#include "InteractiveMarker.h"

#include "utils.h"

InteractiveMarker::MarkerPositionControl::MarkerPositionControl(Color color, Eigen::Vector3d offset)
: color(color), offset(offset)
{
}

InteractiveMarker::MarkerOrientationControl::MarkerOrientationControl(Model torus, Color c, Vector3 normal, Matrix ori)
: torus(torus), color(c), normal(normal), orientation(ori)
{
}

InteractiveMarker::InteractiveMarker(const sva::PTransformd & pose) : pose_(pose)
{
  posControls_[0] = {RED, Eigen::Vector3d::UnitX()};
  posControls_[1] = {RED, -Eigen::Vector3d::UnitX()};
  posControls_[2] = {GREEN, Eigen::Vector3d::UnitY()};
  posControls_[3] = {GREEN, -Eigen::Vector3d::UnitY()};
  posControls_[4] = {BLUE, Eigen::Vector3d::UnitZ()};
  posControls_[5] = {BLUE, -Eigen::Vector3d::UnitZ()};
  oriControls_[0] = {LoadModelFromMesh(GenMeshTorus(0.1f, 0.3f, 8, 32)), RED, {1, 0, 0}, MatrixRotateY(M_PI / 2)};
  oriControls_[1] = {LoadModelFromMesh(GenMeshTorus(0.1f, 0.3f, 8, 32)), GREEN, {0, 1, 0}, MatrixRotateX(M_PI / 2)};
  oriControls_[2] = {LoadModelFromMesh(GenMeshTorus(0.1f, 0.3f, 8, 32)), BLUE, {0, 0, 1}};
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

void InteractiveMarker::update(Camera camera, Ray ray, SceneState & state)
{
  for(auto & c : posControls_)
  {
    c.hover = CheckCollisionRayBox(ray, c.bbox);
    if(!c.active)
    {
      if(IsMouseButtonPressed(0) && c.hover && state.mouseHandler == nullptr)
      {
        state.mouseHandler = this;
        c.active = true;
        c.start = GetMousePosition();
        Vector2 start = GetWorldToScreen(translation(pose_), camera);
        Vector2 end = GetWorldToScreen(translation(sva::PTransformd(c.offset) * pose_), camera);
        c.dir = Vector2Subtract(end, start);
      }
      else
      {
        c.active = false;
      }
    }
    else
    {
      if(IsMouseButtonReleased(0))
      {
        if(c.active)
        {
          state.mouseHandler = nullptr;
        }
        c.active = false;
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
  }
  for(auto & c : oriControls_)
  {
    auto hit = GetCollisionRayModel(ray, c.torus);
    c.hover = hit.hit;
    if(!c.active)
    {
      if(IsMouseButtonPressed(0) && c.hover && state.mouseHandler == nullptr)
      {
        state.mouseHandler = this;
        c.active = true;
        c.pZ = Vector3Transform(c.normal, convert(pose_.rotation()));
        Vector3 point = intersection(ray, c.pZ, translation(pose_));
        c.pX = Vector3Normalize(Vector3Subtract(point, translation(pose_)));
        c.pY = Vector3Normalize(Vector3CrossProduct(c.pZ, c.pX));
      }
      else
      {
        c.active = false;
      }
    }
    else
    {
      if(IsMouseButtonReleased(0))
      {
        if(c.active)
        {
          state.mouseHandler = nullptr;
        }
        c.active = false;
      }
      else
      {
        Vector3 point = Vector3Normalize(Vector3Subtract(intersection(ray, c.pZ, translation(pose_)), translation(pose_)));
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
  }
}

void InteractiveMarker::draw()
{
  for(auto & c : posControls_)
  {
    DrawCube(c.pose, size, size, size, c.color);
    if(c.hover || c.active)
    {
      DrawCubeWires(c.pose, size, size, size, c.active ? WHITE : GRAY);
    }
    if(c.active)
    {
      DrawCubeWires(c.pose, size, size, size, WHITE);
    }
  }
  for(auto & c : oriControls_)
  {
    DrawModel(c.torus, Vector3Zero(), 1.0f, c.color);
    if(c.hover || c.active)
    {
      DrawModelWires(c.torus, Vector3Zero(), 1.0f, c.active ? WHITE : GRAY);
      if(c.active)
      {
        DrawLine3D(translation(pose_), Vector3Add(translation(pose_), c.pX), RED);
        DrawLine3D(translation(pose_), Vector3Add(translation(pose_), c.pY), GREEN);
        DrawLine3D(translation(pose_), Vector3Add(translation(pose_), c.pZ), BLUE);
      }
    }
  }
}
