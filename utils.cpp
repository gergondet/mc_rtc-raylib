#include "utils.h"

#include "raylib.h"
#include "rlgl.h"

Matrix convert(const sva::PTransformd & pt)
{
  auto mat = sva::conversions::toHomogeneous(pt.cast<float>());
  Matrix out;
  out.m0 = mat(0, 0);
  out.m1 = mat(1, 0);
  out.m2 = mat(2, 0);
  out.m3 = mat(3, 0);
  out.m4 = mat(0, 1);
  out.m5 = mat(1, 1);
  out.m6 = mat(2, 1);
  out.m7 = mat(3, 1);
  out.m8 = mat(0, 2);
  out.m9 = mat(1, 2);
  out.m10 = mat(2, 2);
  out.m11 = mat(3, 2);
  out.m12 = mat(0, 3);
  out.m13 = mat(1, 3);
  out.m14 = mat(2, 3);
  out.m15 = mat(3, 3);
  return out;
}

Vector3 translation(const sva::PTransformd & pt)
{
  const auto & t = pt.translation().cast<float>();
  return {t.x(), t.y(), t.z()};
}

Vector3 intersection(Ray ray, Vector3 normal, Vector3 point)
{
  float n_d = Vector3DotProduct(normal, ray.direction);
  if(n_d == 0.0f)
  {
    printf("NO INTERSECTION\n");
    return point;
  }
  float t = (Vector3DotProduct(normal, point) - Vector3DotProduct(normal, ray.position)) / n_d;
  return Vector3Add(ray.position, Vector3Scale(ray.direction, t));
}

void DrawGridXY(int slices, float spacing)
{
  int halfSlices = slices / 2;

  if(rlCheckBufferLimit(slices * 4)) rlglDraw();

  rlBegin(RL_LINES);
  for(int i = -halfSlices; i <= halfSlices; i++)
  {
    if(i == 0)
    {
      rlColor3f(0.5f, 0.5f, 0.5f);
      rlColor3f(0.5f, 0.5f, 0.5f);
      rlColor3f(0.5f, 0.5f, 0.5f);
      rlColor3f(0.5f, 0.5f, 0.5f);
    }
    else
    {
      rlColor3f(0.75f, 0.75f, 0.75f);
      rlColor3f(0.75f, 0.75f, 0.75f);
      rlColor3f(0.75f, 0.75f, 0.75f);
      rlColor3f(0.75f, 0.75f, 0.75f);
    }

    rlVertex3f((float)i * spacing, (float)-halfSlices * spacing, 0.0f);
    rlVertex3f((float)i * spacing, (float)halfSlices * spacing, 0.0f);

    rlVertex3f((float)-halfSlices * spacing, (float)i * spacing, 0.0f);
    rlVertex3f((float)halfSlices * spacing, (float)i * spacing, 0.0f);
  }
  rlEnd();
}
