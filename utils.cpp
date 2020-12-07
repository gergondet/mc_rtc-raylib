#include "utils.h"

#ifndef M_PI
#  define M_PI 3.14159265358979323846264338327950288
#endif

#include "raylib.h"
#include "rlgl.h"

#define PAR_MALLOC(T, N) ((T *)RL_MALLOC(N * sizeof(T)))
#define PAR_CALLOC(T, N) ((T *)RL_CALLOC(N * sizeof(T), 1))
#define PAR_REALLOC(T, BUF, N) ((T *)RL_REALLOC(BUF, sizeof(T) * (N)))
#define PAR_FREE RL_FREE

#include "raylib/src/external/par_shapes.h"

#ifndef DEFAULT_MESH_VERTEX_BUFFERS
#  define DEFAULT_MESH_VERTEX_BUFFERS 7
#endif

#include "imgui.h"

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

void DrawCylinderEx(Vector3 position,
                    Vector3 normal,
                    float radiusTop,
                    float radiusBottom,
                    float height,
                    int sides,
                    Color color)
{
  if(sides < 3) sides = 3;
  normal = Vector3Normalize(normal);

  int numVertex = sides * 6;
  if(rlCheckBufferLimit(numVertex)) rlglDraw();

  rlPushMatrix();
  {
    rlLoadIdentity();
    rlTranslatef(position.x, position.y, position.z);
    auto vec = Vector3CrossProduct({0, 1, 0}, normal);
    rlRotatef(RAD2DEG * acosf(Vector3DotProduct({0, 1, 0}, normal)), vec.x, vec.y, vec.z);

    rlBegin(RL_TRIANGLES);
    {
      rlColor4ub(color.r, color.g, color.b, color.a);

      if(radiusTop > 0)
      {
        // Draw Body
        for(int i = 0; i < 360; i += 360 / sides)
        {
          rlVertex3f(sinf(DEG2RAD * i) * radiusBottom, 0, cosf(DEG2RAD * i) * radiusBottom); // Bottom Left
          rlVertex3f(sinf(DEG2RAD * (i + 360 / sides)) * radiusBottom, 0,
                     cosf(DEG2RAD * (i + 360 / sides)) * radiusBottom); // Bottom Right
          rlVertex3f(sinf(DEG2RAD * (i + 360 / sides)) * radiusTop, height,
                     cosf(DEG2RAD * (i + 360 / sides)) * radiusTop); // Top Right

          rlVertex3f(sinf(DEG2RAD * i) * radiusTop, height, cosf(DEG2RAD * i) * radiusTop); // Top Left
          rlVertex3f(sinf(DEG2RAD * i) * radiusBottom, 0, cosf(DEG2RAD * i) * radiusBottom); // Bottom Left
          rlVertex3f(sinf(DEG2RAD * (i + 360 / sides)) * radiusTop, height,
                     cosf(DEG2RAD * (i + 360 / sides)) * radiusTop); // Top Right
        }

        // Draw Cap
        for(int i = 0; i < 360; i += 360 / sides)
        {
          rlVertex3f(0, height, 0);
          rlVertex3f(sinf(DEG2RAD * i) * radiusTop, height, cosf(DEG2RAD * i) * radiusTop);
          rlVertex3f(sinf(DEG2RAD * (i + 360 / sides)) * radiusTop, height,
                     cosf(DEG2RAD * (i + 360 / sides)) * radiusTop);
        }
      }
      else
      {
        // Draw Cone
        for(int i = 0; i < 360; i += 360 / sides)
        {
          rlVertex3f(0, height, 0);
          rlVertex3f(sinf(DEG2RAD * i) * radiusBottom, 0, cosf(DEG2RAD * i) * radiusBottom);
          rlVertex3f(sinf(DEG2RAD * (i + 360 / sides)) * radiusBottom, 0,
                     cosf(DEG2RAD * (i + 360 / sides)) * radiusBottom);
        }
      }

      // Draw Base
      for(int i = 0; i < 360; i += 360 / sides)
      {
        rlVertex3f(0, 0, 0);
        rlVertex3f(sinf(DEG2RAD * (i + 360 / sides)) * radiusBottom, 0,
                   cosf(DEG2RAD * (i + 360 / sides)) * radiusBottom);
        rlVertex3f(sinf(DEG2RAD * i) * radiusBottom, 0, cosf(DEG2RAD * i) * radiusBottom);
      }
    }
    rlEnd();
  }
  rlPopMatrix();
}

void DrawArrow(Vector3 p0, Vector3 p1, float shaft_diam, float head_diam, float head_len, Color color)
{
  Vector3 normal = Vector3Subtract(p1, p0);
  float height = Vector3Length(normal);
  if(height == 0.0f)
  {
    return;
  }
  normal = Vector3Scale(normal, 1 / height);
  if(head_len >= height)
  {
    head_len = height;
  }
  float shaft_len = height - head_len;
  if(shaft_len != 0)
  {
    DrawCylinderEx(p0, normal, shaft_diam / 2, shaft_diam / 2, shaft_len, 8, color);
  }
  DrawCylinderEx(Vector3Add(p0, Vector3Scale(normal, shaft_len)), normal, 0, head_diam / 2, head_len, 8, color);
}

void DrawFrame(const sva::PTransformd & pose)
{
  auto px = translation(sva::PTransformd(Eigen::Vector3d{0.15, 0, 0}) * pose);
  auto py = translation(sva::PTransformd(Eigen::Vector3d{0, 0.15, 0}) * pose);
  auto pz = translation(sva::PTransformd(Eigen::Vector3d{0, 0, 0.15}) * pose);
  auto p0 = translation(pose);
  DrawArrow(p0, px, 0.15 * 0.15, 0.15 * 0.15, 0.5 * 0.15, RED);
  DrawArrow(p0, py, 0.15 * 0.15, 0.15 * 0.15, 0.5 * 0.15, GREEN);
  DrawArrow(p0, pz, 0.15 * 0.15, 0.15 * 0.15, 0.5 * 0.15, BLUE);
}

void Combo(const char * label,
           const std::vector<std::string> & values,
           const std::string & current,
           std::function<void(const std::string &)> callback)
{
  if(ImGui::BeginCombo(label, current.c_str()))
  {
    for(const auto & v : values)
    {
      bool active = v == current;
      if(ImGui::Selectable(v.c_str(), active))
      {
        callback(v);
      }
      if(active)
      {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
}

void Combo(const char * label, const std::vector<std::string> & values, std::string & current)
{
  Combo(label, values, current, [&current](const std::string & v) { current = v; });
}

Mesh GenMeshCylinderROS(float radius, float height, int slices)
{
  Mesh mesh = {};
  mesh.vboId = (unsigned int *)RL_CALLOC(DEFAULT_MESH_VERTEX_BUFFERS, sizeof(unsigned int));

  // Generate the cylinder body
  par_shapes_mesh * cylinder = par_shapes_create_cylinder(slices, 8);
  par_shapes_scale(cylinder, radius, radius, height);
  par_shapes_translate(cylinder, 0, 0, -height / 2);

  // Generate an orientable disk shape (top cap)
  float cap_center[3] = {0, 0, height / 2};
  float cap_normal[3] = {0, 0, 1};
  par_shapes_mesh * capTop = par_shapes_create_disk(radius, slices, cap_center, cap_normal);
  capTop->tcoords = PAR_MALLOC(float, 2 * capTop->npoints);
  for(int i = 0; i < 2 * capTop->npoints; i++) capTop->tcoords[i] = 0.0f;

  par_shapes_merge(cylinder, capTop);

  // Generate an orientable disk shape (bottom cap)
  float rot_axis[3] = {1, 0, 0};
  par_shapes_rotate(capTop, M_PI, rot_axis);

  par_shapes_merge_and_free(cylinder, capTop);

  mesh.vertices = (float *)RL_MALLOC(cylinder->ntriangles * 3 * 3 * sizeof(float));
  mesh.texcoords = (float *)RL_MALLOC(cylinder->ntriangles * 3 * 2 * sizeof(float));
  mesh.normals = (float *)RL_MALLOC(cylinder->ntriangles * 3 * 3 * sizeof(float));

  mesh.vertexCount = cylinder->ntriangles * 3;
  mesh.triangleCount = cylinder->ntriangles;

  for(int k = 0; k < mesh.vertexCount; k++)
  {
    mesh.vertices[k * 3] = cylinder->points[cylinder->triangles[k] * 3];
    mesh.vertices[k * 3 + 1] = cylinder->points[cylinder->triangles[k] * 3 + 1];
    mesh.vertices[k * 3 + 2] = cylinder->points[cylinder->triangles[k] * 3 + 2];

    mesh.normals[k * 3] = cylinder->normals[cylinder->triangles[k] * 3];
    mesh.normals[k * 3 + 1] = cylinder->normals[cylinder->triangles[k] * 3 + 1];
    mesh.normals[k * 3 + 2] = cylinder->normals[cylinder->triangles[k] * 3 + 2];

    mesh.texcoords[k * 2] = cylinder->tcoords[cylinder->triangles[k] * 2];
    mesh.texcoords[k * 2 + 1] = cylinder->tcoords[cylinder->triangles[k] * 2 + 1];
  }

  par_shapes_free_mesh(cylinder);

  // Upload vertex data to GPU (static mesh)
  rlLoadMesh(&mesh, false);

  return mesh;
}
