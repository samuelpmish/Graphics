#include <string>

namespace Graphics {

extern const std::string quad8_tcs_shader(R"tcs(
#version 400
#extension GL_ARB_tessellation_shader: enable

layout(vertices = 8) out;

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

extern const std::string quad8_tes_shader(R"tes(
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
  float weights[8];
  weights[0] = -((-1 + eta)*(-1 + xi)*(-1 + 2*eta + 2*xi));
  weights[1] = (-1 + eta)*(1 + 2*eta - 2*xi)*xi;
  weights[2] = eta*xi*(-3 + 2*eta + 2*xi);
  weights[3] = -(eta*(-1 + 2*eta - 2*xi)*(-1 + xi));
  weights[4] = 4*(-1 + eta)*(-1 + xi)*xi;
  weights[5] = -4*(-1 + eta)*eta*xi;
  weights[6] = -4*eta*(-1 + xi)*xi;
  weights[7] = 4*(-1 + eta)*eta*(-1 + xi);

  vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
  vec3 position = vec3(0.0, 0.0, 0.0);
  for (int i = 0; i < 8; i++) {
    position += weights[i] * inData[i].position;
    color    += weights[i] * inData[i].color;
  }

  gl_Position = proj * vec4(position, 1.0);
  outData.color = color;

}
)tes");

}