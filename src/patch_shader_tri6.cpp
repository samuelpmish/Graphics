#include <string>

namespace Graphics {

extern const std::string tri6_tcs_shader(R"tcs(
#version 400
#extension GL_ARB_tessellation_shader: enable

layout(vertices = 6) out;

in vertexData {
  vec3 position;
  float value;
} inData[];

out tessData {
  vec3 position;
  float value;
} outData[];

const float quality =  15.0;

void main() {

  gl_TessLevelOuter[0] = quality;
  gl_TessLevelOuter[1] = quality;
  gl_TessLevelOuter[2] = quality;
  gl_TessLevelOuter[3] = quality;

  gl_TessLevelInner[0] = quality;
  gl_TessLevelInner[1] = quality;

  outData[gl_InvocationID].position = inData[gl_InvocationID].position;
  outData[gl_InvocationID].value    = inData[gl_InvocationID].value;

}
)tcs");

extern const std::string tri6_tes_shader(R"tes(
#version 400
#extension GL_ARB_tessellation_shader: enable

layout(quads, equal_spacing) in;

in tessData{
  vec3 position;
  float value;
} inData[];

out fragData{
  float value;
} outData;

uniform mat4 proj;

void main() {

  float xi = gl_TessCoord.x;
  float eta = gl_TessCoord.y;

  // evaluate the quad4 shape functions
  float weights[4];
  weights[0] = (1.0 - xi) * (1.0 - eta);
  weights[1] =        xi  * (1.0 - eta);
  weights[2] = (1.0 - xi) *        eta ; 
  weights[3] =        xi  *        eta ;

  float value = 0.0;
  vec3 position = vec3(0.0, 0.0, 0.0);
  for (int i = 0; i < 4; i++) {
    position += weights[i] * inData[i].position;
    value +=    weights[i] * inData[i].value;
  }

  gl_Position = proj * vec4(position, 1.0);

  outData.value = value;

}
)tes");

}