#include "RobotModel.h"

#include "utils.h"

#include "r3d.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <SpaceVecAlg/Conversions.h>

#include <mc_rtc/config.h>

#include <boost/filesystem.hpp>
namespace bfs = boost::filesystem;

#ifdef MC_RTC_HAS_ROS_SUPPORT
#  include <ros/package.h>
#endif

std::string GOOSH_VERTEX_SHADER_ES = R"(
#version 100

// Input vertex attributes
attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec3 vertexNormal;
attribute vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;

// Output vertex attributes (to fragment shader)
varying vec3 fragPosition;
varying vec2 fragTexCoord;
varying vec4 fragColor;
varying vec3 fragNormal;

// https://github.com/glslify/glsl-inverse
mat3 inverse(mat3 m)
{
  float a00 = m[0][0], a01 = m[0][1], a02 = m[0][2];
  float a10 = m[1][0], a11 = m[1][1], a12 = m[1][2];
  float a20 = m[2][0], a21 = m[2][1], a22 = m[2][2];

  float b01 = a22*a11 - a12*a21;
  float b11 = -a22*a10 + a12*a20;
  float b21 = a21*a10 - a11*a20;

  float det = a00*b01 + a01*b11 + a02*b21;

  return mat3(b01, (-a22*a01 + a02*a21), (a12*a01 - a02*a11),
              b11, (a22*a00 - a02*a20), (-a12*a00 + a02*a10),
              b21, (-a21*a00 + a01*a20), (a11*a00 - a01*a10))/det;
}

// https://github.com/glslify/glsl-transpose
mat3 transpose(mat3 m)
{
  return mat3(m[0][0], m[1][0], m[2][0],
              m[0][1], m[1][1], m[2][1],
              m[0][2], m[1][2], m[2][2]);
}

void main()
{
    // Send vertex attributes to fragment shader
    fragPosition = vec3(matModel*vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    mat3 normalMatrix = transpose(inverse(mat3(matModel)));
    fragNormal = normalize(normalMatrix*vertexNormal);

    // Calculate final vertex position
    gl_Position = mvp*vec4(vertexPosition, 1.0);
}
)";

std::string GOOSH_VERTEX_SHADER = R"(
#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

void main()
{
    // Send vertex attributes to fragment shader
    fragPosition = vec3(matModel*vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    mat3 normalMatrix = transpose(inverse(mat3(matModel)));
    fragNormal = normalize(normalMatrix*vertexNormal);

    // Calculate final vertex position
    gl_Position = mvp*vec4(vertexPosition, 1.0);
}
)";

// Fragment shader from https://rendermeapangolin.wordpress.com/2015/05/07/gooch-shading/

std::string GOOSH_FRAGMENT_SHADER_ES = R"(
#version 100

precision mediump float;

// Vertex attributes
varying vec3 fragPosition;
varying vec2 fragTexCoord;
varying vec4 fragColor;
varying vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 viewPos;
uniform vec3 targetPos;

vec4 gooch_shading(vec4 m_color, //color of the mesh
                   float m_shine, //shininess of the surface
                   vec3 l_direction, //light direction
                   vec3 v_normal, //normal
                   vec3 c_direction) //camera direction
{
    //diffuse
    float kd = 1.0;
    float a = 0.1;
    float b = 0.9;

    float NL = dot(normalize(v_normal), normalize(l_direction));

    float it = ((1.0 + NL) / 2.0);
    vec3 color = (1.0-it) * (vec3(0.0, 0.0, 0.2) + a*m_color.xyz)
               +  it * (vec3(0.2, 0.2, 0.0) + b*m_color.xyz);

    //Highlights
    vec3 R = reflect( -normalize(l_direction),
                      normalize(v_normal) );
    float ER = clamp( dot( normalize(c_direction),
                           normalize(R)),
                     0.0, 1.0);

    vec4 spec = vec4(1.0) * pow(ER, m_shine);

    return vec4(color+spec.xyz, m_color.a);
}

void main()
{
  gl_FragColor = texture2D(texture0, fragTexCoord) * colDiffuse * fragColor;
  gl_FragColor = gooch_shading(gl_FragColor,
                               10.0,
                               vec3(0, 0, 1),
                               fragNormal,
                               normalize(viewPos - targetPos));
}
)";

std::string GOOSH_FRAGMENT_SHADER = R"(
#version 330

// Vertex attributes
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 viewPos;
uniform vec3 targetPos;

// Output color
out vec4 finalColor;

vec4 gooch_shading(vec4 m_color, //color of the mesh
                   float m_shine, //shininess of the surface
                   vec3 l_direction, //light direction
                   vec3 v_normal, //normal
                   vec3 c_direction) //camera direction
{
    //diffuse
    float kd = 1;
    float a = 0.1;
    float b = 0.9;

    float NL = dot(normalize(v_normal), normalize(l_direction));

    float it = ((1 + NL) / 2);
    vec3 color = (1-it) * (vec3(0, 0, 0.2) + a*m_color.xyz)
               +  it * (vec3(0.2, 0.2, 0) + b*m_color.xyz);

    //Highlights
    vec3 R = reflect( -normalize(l_direction),
                      normalize(v_normal) );
    float ER = clamp( dot( normalize(c_direction),
                           normalize(R)),
                     0, 1);

    vec4 spec = vec4(1) * pow(ER, m_shine);

    return vec4(color+spec.xyz, m_color.a);
}

void main()
{
  finalColor = texture(texture0, fragTexCoord) * colDiffuse * fragColor;
  finalColor = gooch_shading(finalColor,
                             10.0,
                             vec3(0, 0, 1),
                             fragNormal,
                             normalize(viewPos - targetPos));
}
)";

bfs::path convertURI(const std::string & uri)
{
  const std::string package = "package://";
  if(uri.size() >= package.size() && uri.find(package) == 0)
  {
    size_t split = uri.find('/', package.size());
    std::string pkg = uri.substr(package.size(), split - package.size());
    auto leaf = bfs::path(uri.substr(split + 1));
    bfs::path MC_ENV_DESCRIPTION_PATH(mc_rtc::MC_ENV_DESCRIPTION_PATH);
#ifndef MC_RTC_HAS_ROS_SUPPORT
    // FIXME Prompt the user for unknown packages
    if(pkg == "jvrc_description")
    {
      pkg = (MC_ENV_DESCRIPTION_PATH / ".." / "jvrc_description").string();
    }
    else if(pkg == "mc_env_description")
    {
      pkg = MC_ENV_DESCRIPTION_PATH.string();
    }
    else if(pkg == "mc_int_obj_description")
    {
      pkg = (MC_ENV_DESCRIPTION_PATH / ".." / "mc_int_obj_description").string();
    }
    else
    {
      mc_rtc::log::critical("Cannot resolve package: {}", pkg);
    }
#else
    pkg = ros::package::getPath(pkg);
#endif
    return pkg / leaf;
  }
  const std::string file = "file://";
  if(uri.size() >= file.size() && uri.find(file) == 0)
  {
    return bfs::path(uri.substr(file.size()));
  }
  return uri;
}

BodyDrawer::ModelData::~ModelData()
{
  UnloadModel(model);
}

BodyDrawer::BodyDrawer(const std::vector<rbd::parsers::Visual> & v, Shader * shader)
{
  auto fromMesh = [&](const rbd::parsers::Visual & v) {
    const auto & mesh = boost::get<rbd::parsers::Geometry::Mesh>(v.geometry.data);
    auto path = convertURI(mesh.filename);
#ifndef __EMSCRIPTEN__
    auto cwd = bfs::current_path();
    bfs::current_path(path.parent_path());
#else
    auto cwd = get_current_dir_name();
    chdir(path.parent_path().string().c_str());
#endif
    if(path.extension() == ".obj" || path.extension() == ".gltf" || path.extension() == ".glb")
    {
      models_.emplace_back(new ModelData{LoadModel(path.string().c_str()), path.leaf().string(),
                                         static_cast<float>(mesh.scale), WHITE, v.origin});
    }
    else
    {
      models_.emplace_back(new ModelData{LoadModelAdvanced(path.string().c_str()), path.leaf().string(),
                                         static_cast<float>(mesh.scale), WHITE, v.origin});
    }
    if(v.material.type == rbd::parsers::Material::Type::COLOR)
    {
      const auto & c = boost::get<rbd::parsers::Material::Color>(v.material.data);
      auto isWhite = [&]() { return c.r == c.g && c.g == c.b && c.b == c.a && c.a == 1.0; };
      if(!isWhite())
      {
        shader = nullptr;
        models_.back()->color = ColorFromNormalized(
            {static_cast<float>(c.r), static_cast<float>(c.g), static_cast<float>(c.b), static_cast<float>(c.a)});
      }
    }
    else if(v.material.type == rbd::parsers::Material::Type::TEXTURE)
    {
      const auto & textureIn = boost::get<rbd::parsers::Material::Texture>(v.material.data);
      auto filename = convertURI(textureIn.filename).string();
      auto texture = LoadTexture(filename.c_str());
      models_.back()->model.materials[0].maps[MAP_ALBEDO].texture = texture;
      shader = nullptr;
    }
    if(shader)
    {
      for(int i = 0; i < models_.back()->model.materialCount; ++i)
      {
        models_.back()->model.materials[i].shader = *shader;
      }
    }
#ifndef __EMSCRIPTEN__
    bfs::current_path(cwd);
#else
    chdir(cwd);
    free(cwd);
#endif
  };
  for(const auto & visual : v)
  {
    switch(visual.geometry.type)
    {
      case rbd::parsers::Geometry::MESH:
        fromMesh(visual);
        break;
      default:
        break;
    }
  }
}

void BodyDrawer::update(const sva::PTransformd & pose)
{
  for(auto & m : models_)
  {
    m->model.transform = convert(m->X_b_model * pose);
  }
}

void BodyDrawer::draw()
{
  for(const auto & m : models_)
  {
    DrawModel(m->model, {0, 0, 0}, m->scale, m->color);
  }
}

RobotModel::RobotModel(const mc_rbdyn::Robot & robot, bool useCollisionModel)
{
  const auto & visual = useCollisionModel ? robot.module()._collision : robot.module()._visual;
#ifndef __EMSCRIPTEN__
  shader_ = LoadShaderCode(GOOSH_VERTEX_SHADER.c_str(), GOOSH_FRAGMENT_SHADER.c_str());
#else
  shader_ = LoadShaderCode(GOOSH_VERTEX_SHADER_ES.c_str(), GOOSH_FRAGMENT_SHADER_ES.c_str());
#endif
  shader_.locs[LOC_MATRIX_MODEL] = GetShaderLocation(shader_, "matModel");
  shader_.locs[LOC_VECTOR_VIEW] = GetShaderLocation(shader_, "viewPos");
  for(const auto & b : robot.mb().bodies())
  {
    if(!visual.count(b.name()))
    {
      bodies_.push_back({{}, nullptr});
    }
    else
    {
      bodies_.emplace_back(visual.at(b.name()), &shader_);
    }
  }
}

void RobotModel::update(const mc_rbdyn::Robot & robot)
{
  const auto & poses = robot.bodyPosW();
  for(size_t i = 0; i < poses.size(); ++i)
  {
    bodies_[i].update(poses[i]);
  }
}

void RobotModel::draw(Camera camera)
{
  float cameraPos[3] = {camera.position.x, camera.position.y, camera.position.z};
  SetShaderValue(shader_, shader_.locs[LOC_VECTOR_VIEW], cameraPos, UNIFORM_VEC3);
  float targetPos[3] = {camera.target.x, camera.target.y, camera.target.z};
  SetShaderValue(shader_, GetShaderLocation(shader_, "targetPos"), targetPos, UNIFORM_VEC3);
  for(auto & b : bodies_)
  {
    b.draw();
  }
}

RobotModel::~RobotModel()
{
  UnloadShader(shader_);
}
