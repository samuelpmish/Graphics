#include <string>

namespace Graphics {

extern const std::string quad9_tcs_shader(R"tcs(
#version 400
#extension GL_ARB_tessellation_shader: enable

layout(vertices = 9) out;

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

extern const std::string quad9_tes_shader(R"tes(
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

  float w_xi[3];
  w_xi[0] = (-1 + xi)*(-1 + 2*xi);
  w_xi[1] = -4*(-1 + xi)*xi;
  w_xi[2] = xi*(-1 + 2*xi);

  float w_eta[3];
  w_eta[0] = (-1 + eta)*(-1 + 2*eta);
  w_eta[1] = -4*(-1 + eta)*eta;
  w_eta[2] = eta*(-1 + 2*eta);

  vec3 position = inData[0].position * w_xi[0] * w_eta[0] + 
                  inData[1].position * w_xi[2] * w_eta[0] + 
                  inData[2].position * w_xi[2] * w_eta[2] + 
                  inData[3].position * w_xi[0] * w_eta[2] + 
                  inData[4].position * w_xi[1] * w_eta[0] + 
                  inData[5].position * w_xi[2] * w_eta[1] + 
                  inData[6].position * w_xi[1] * w_eta[2] + 
                  inData[7].position * w_xi[0] * w_eta[1] + 
                  inData[8].position * w_xi[1] * w_eta[1];

  vec4 color = inData[0].color * w_xi[0] * w_eta[0] + 
               inData[1].color * w_xi[2] * w_eta[0] + 
               inData[2].color * w_xi[2] * w_eta[2] + 
               inData[3].color * w_xi[0] * w_eta[2] + 
               inData[4].color * w_xi[1] * w_eta[0] + 
               inData[5].color * w_xi[2] * w_eta[1] + 
               inData[6].color * w_xi[1] * w_eta[2] + 
               inData[7].color * w_xi[0] * w_eta[1] + 
               inData[8].color * w_xi[1] * w_eta[1];

  gl_Position = proj * vec4(position, 1.0);
  outData.color = color;

}
)tes");

}