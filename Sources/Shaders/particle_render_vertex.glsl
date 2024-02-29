#version 150 core

in vec2 position;
in float age;
in float life;
in vec2 velocity;

void main() {
  gl_PointSize = 1.0;
  gl_Position = vec4(position, 0.0, 1.0);
}
