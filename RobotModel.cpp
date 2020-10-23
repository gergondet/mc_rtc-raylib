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
    float a = 0.2;
    float b = 0.6;

    float NL = dot(normalize(v_normal), normalize(l_direction));

    float it = ((1 + NL) / 2);
    vec3 color = (1-it) * (vec3(0, 0, 0.4) + a*m_color.xyz)
               +  it * (vec3(0.4, 0.4, 0) + b*m_color.xyz);

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
                             0.5,
                             vec3(0, 0, 1),
                             fragNormal,
                             normalize(viewPos));
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
    // FIXME Use ROS if available, prompt the user otherwise
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
    return pkg / leaf;
  }
}

BodyDrawer::ModelData::~ModelData()
{
  UnloadModel(model);
}

BodyDrawer::BodyDrawer(const std::vector<rbd::parsers::Visual> & v, Shader shader)
{
  auto fromMesh = [&](const rbd::parsers::Visual & v) {
    const auto & mesh = boost::get<rbd::parsers::Geometry::Mesh>(v.geometry.data);
    auto path = convertURI(mesh.filename);
    auto cwd = bfs::current_path();
    bfs::current_path(path.parent_path());
    models_.emplace_back(new ModelData{LoadModelAdvanced(path.string().c_str()), path.leaf().string(),
                                       static_cast<float>(mesh.scale), v.origin});
    for(int i = 0; i < models_.back()->model.materialCount; ++i)
    {
      models_.back()->model.materials[i].shader = shader;
    }
    bfs::current_path(cwd);
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
    DrawModel(m->model, {0, 0, 0}, m->scale, WHITE);
  }
}

RobotModel::RobotModel(const mc_rbdyn::Robot & robot, bool useCollisionModel)
{
  const auto & visual = useCollisionModel ? robot.module()._collision : robot.module()._visual;
  shader_ = LoadShaderCode(GOOSH_VERTEX_SHADER.c_str(), GOOSH_FRAGMENT_SHADER.c_str());
  shader_.locs[LOC_MATRIX_MODEL] = GetShaderLocation(shader_, "matModel");
  shader_.locs[LOC_VECTOR_VIEW] = GetShaderLocation(shader_, "viewPos");
  for(const auto & b : robot.mb().bodies())
  {
    if(!visual.count(b.name()))
    {
      bodies_.push_back({{}, shader_});
    }
    else
    {
      bodies_.emplace_back(visual.at(b.name()), shader_);
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
  for(auto & b : bodies_)
  {
    b.draw();
  }
}

RobotModel::~RobotModel()
{
  UnloadShader(shader_);
}
