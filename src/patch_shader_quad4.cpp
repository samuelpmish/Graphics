#include <string>

namespace Graphics {

extern const std::string quad4_tcs_shader(R"tcs(
#version 400
#extension GL_ARB_tessellation_shader: enable

layout(vertices = 4) out;

in vertexData {
  vec3 position;
  vec4 color;
} inData[];

out tessData {
  vec3 position;
  vec4 color;
} outData[];

uniform float subdivision;

void main() {

  gl_TessLevelOuter[0] = subdivision;
  gl_TessLevelOuter[1] = subdivision;
  gl_TessLevelOuter[2] = subdivision;
  gl_TessLevelOuter[3] = subdivision;

  gl_TessLevelInner[0] = subdivision;
  gl_TessLevelInner[1] = subdivision;

  outData[gl_InvocationID].position = inData[gl_InvocationID].position;
  outData[gl_InvocationID].color    = inData[gl_InvocationID].color;

}
)tcs");

extern const std::string quad4_tes_shader(R"tes(
#version 400
#extension GL_ARB_tessellation_shader: enable

layout(quads, equal_spacing) in;

in tessData{
  vec3 position;
  vec4 color;
} inData[];

out fragData{
  vec4 color;
} outData;

uniform mat4 proj;

void main() {

  float xi = gl_TessCoord.x;
  float eta = gl_TessCoord.y;

  // evaluate the quad4 shape functions
  float weights[4];
  weights[0] = (1.0 - xi) * (1.0 - eta);
  weights[1] =        xi  * (1.0 - eta);
  weights[2] =        xi  *        eta ;
  weights[3] = (1.0 - xi) *        eta ; 

  vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
  vec3 position = vec3(0.0, 0.0, 0.0);
  for (int i = 0; i < 4; i++) {
    position += weights[i] * inData[i].position;
    color    += weights[i] * inData[i].color;
  }

  gl_Position = proj * vec4(position, 1.0);
  outData.color = color;

}
)tes");

}