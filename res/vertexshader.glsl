#version 330 core

layout (location = 0) in vec3 m_position;

void main()
{
  gl_Position = vec4(m_position.x, m_position.y, m_position.z, 1.0f);
}
