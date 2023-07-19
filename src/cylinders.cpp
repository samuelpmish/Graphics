#include "cylinders.hpp"

#include <string>
#include <iostream>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_operation.hpp>

#include "glError.hpp"

static constexpr int angular_divisions = 16;

static const std::vector< glm::vec3 > cylinder_vertices = [](){
  std::vector< glm::vec3 > vertices(2 * (angular_divisions + 1));
  for (int i = 0; i <= angular_divisions; i++) {
    float theta = i * (2.0 * M_PI / angular_divisions);
    float c = cos(theta);
    float s = sin(theta);
    vertices[2*i+0] = {c, s, 0.0};
    vertices[2*i+1] = {c, s, 1.0};
  }
  return vertices;
}();

#if 1
const std::string vert_shader(R"vert(
#version 400

in vec4 cyl_start;
in vec4 cyl_end;
in vec3 corners;
in vec4 rgba;

out vec4 cylinder_color;

uniform mat4 proj;

void main() {
  cylinder_color = rgba;

  vec3 e3 = cyl_end.xyz - cyl_start.xyz;
  vec3 e1 = normalize(cross(e3, vec3(0,0,1)));
  vec3 e2 = normalize(cross(e3, e1));
  float r = cyl_start.w + corners.z * (cyl_end.w - cyl_start.w);

  gl_Position = proj * vec4(cyl_start.xyz + r * corners.x * e1 + r * corners.y * e2 + corners.z * e3, 1);
}
)vert");

const std::string frag_shader(R"frag(
#version 400

in vec4 cylinder_color;

out vec4 frag_color;

void main() {
  frag_color = cylinder_color;
}
)frag");

#else

const std::string vert_shader(R"vert(
#version 400

in vec4 cyl_start;
in vec4 cyl_end;
in vec3 corners;

out vec4 pa;
out vec4 pb;
out vec3 position;

uniform mat4 proj;

void main() {
  pa = cyl_start;
  pb = cyl_end;

  vec3 mid = 0.5 * (cyl_end.xyz + cyl_start.xyz);
  vec3 e1 = 0.5 * (cyl_end.xyz - cyl_start.xyz);
  vec3 e2 = normalize(cross(e1, vec3(0,0,1)));
  vec3 e3 = normalize(cross(e1, e2));
  float r = 0.5 * ((1 + corners.x) * cyl_start.w + (1 - corners.x) * cyl_end.w);

  position = mid - corners.x * e1 + r * corners.y * e2 + r * corners.z * e3;

  gl_Position = proj * vec4(position, 1);
}
)vert");

const std::string frag_shader(R"frag(
#version 400

in vec4 pa;
in vec4 pb;
in vec3 position;

out vec4 frag_color;

uniform vec3 camera_position;
uniform mat4 proj;

float dot2(in vec3 v) { return dot(v,v); }

// cone defined by extremes pa and pb, and radious ra and rb
// Only one square root and one division is emplyed in the worst case. dot2(v) is dot(v,v)
vec4 coneIntersect( in vec3  ro, in vec3  rd, in vec3  pa, in vec3 pb, in float ra, in float rb )
{
    vec3  ba = pb - pa;
    vec3  oa = ro - pa;
    vec3  ob = ro - pb;
    float m0 = dot(ba,ba);
    float m1 = dot(oa,ba);
    float m2 = dot(rd,ba);
    float m3 = dot(rd,oa);
    float m5 = dot(oa,oa);
    float m9 = dot(ob,ba); 
    
    // caps
    if( m1<0.0 )
    {
        if( dot2(oa*m2-rd*m1)<(ra*ra*m2*m2) ) // delayed division
            return vec4(-m1/m2,-ba*inversesqrt(m0));
    }
    else if( m9>0.0 )
    {
    	float t = -m9/m2;                     // NOT delayed division
        if( dot2(ob+rd*t)<(rb*rb) )
            return vec4(t,ba*inversesqrt(m0));
    }
    
    // body
    float rr = ra - rb;
    float hy = m0 + rr*rr;
    float k2 = m0*m0    - m2*m2*hy;
    float k1 = m0*m0*m3 - m1*m2*hy + m0*ra*(rr*m2*1.0        );
    float k0 = m0*m0*m5 - m1*m1*hy + m0*ra*(rr*m1*2.0 - m0*ra);
    float h = k1*k1 - k2*k0;
    if( h<0.0 ) return vec4(-1.0); //no intersection
    float t = (-k1-sqrt(h))/k2;
    float y = m1 + t*m2;
    if( y<0.0 || y>m0 ) return vec4(-1.0); //no intersection
    return vec4(t, normalize(m0*(m0*(oa+t*rd)+rr*ba*ra)-ba*hy*y));
}

void main() {

  //frag_color = vec4(1.0, 1.0, 1.0, 1.0);

  vec3 ro = camera_position;
  vec3 rd = position - camera_position;

  vec4 hit = coneIntersect(ro, rd, pa.xyz, pb.xyz, pa.w, pb.w);

  float t = hit.x;
  vec3 n = hit.yzw; 
  if (t > 0) {
    frag_color = vec4(1.0, 0.0, 0.0, 1.0);
    vec4 clip = proj * vec4(ro + normalize(rd) * t, 1.0);
    gl_FragDepth = clip.z / clip.w;
  } else {
    frag_color.a = 0.0;
    gl_FragDepth = gl_DepthRange.far;
  }

}
)frag");
#endif

Cylinders::Cylinders() : program({
    Shader::fromString(vert_shader, GL_VERTEX_SHADER),
    Shader::fromString(frag_shader, GL_FRAGMENT_SHADER)
  }), color{255, 255, 255, 255} {

  dirty = true;

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glCheckError(__FILE__, __LINE__);

  glGenBuffers(1, &instance_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * cylinder_vertices.size(), &cylinder_vertices[0], GL_STATIC_DRAW);
  program.setAttribute("corners", 3, sizeof(glm::vec3), 0);
  glCheckError(__FILE__, __LINE__);

  glGenBuffers(1, &cylinder_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, cylinder_vbo);

  program.setAttribute("cyl_start", 4, 2 * sizeof(glm::vec4), 0);
  glVertexAttribDivisor(program.attribute("cyl_start"), 1);

  program.setAttribute("cyl_end", 4, 2 * sizeof(glm::vec4), 16);
  glVertexAttribDivisor(program.attribute("cyl_end"), 1);

  glGenBuffers(1, &color_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
  program.setAttribute("rgba", 4, sizeof(rgbcolor), 0, GL_TRUE, GL_UNSIGNED_BYTE);
  glVertexAttribDivisor(program.attribute("rgba"), 1);

}

void Cylinders::clear() {
  data.clear();
  dirty = true;
}

void Cylinders::append(const std::vector< Cylinder > & more_cylinders) {
  data.reserve(data.size() + more_cylinders.size());
  data.insert(data.end(), more_cylinders.begin(), more_cylinders.end());  

  colors.insert(colors.end(), more_cylinders.size(), color);
  dirty = true;
}

void Cylinders::append(const std::vector< Cylinder > & more_cylinders,
                     const std::vector< rgbcolor > & more_colors) {
  data.reserve(data.size() + more_cylinders.size());
  data.insert(data.end(), more_cylinders.begin(), more_cylinders.end());  

  colors.reserve(colors.size() + more_colors.size());
  colors.insert(colors.end(), more_colors.begin(), more_colors.end());  
  dirty = true;
}

void Cylinders::draw(const Camera & camera) {

  program.use();

  program.setUniform("proj", camera.matrix());
  glCheckError(__FILE__, __LINE__);

  glBindVertexArray(vao);
  if (dirty) {
    glBindBuffer(GL_ARRAY_BUFFER, cylinder_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Cylinder) * data.size(), &data[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rgbcolor) * colors.size(), &colors[0], GL_STATIC_DRAW);
    glCheckError(__FILE__, __LINE__);
    dirty = false;
  }

  //glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glDisable(GL_CULL_FACE);
  glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, cylinder_vertices.size(), data.size());
  glCheckError(__FILE__, __LINE__);

  program.unuse();

}