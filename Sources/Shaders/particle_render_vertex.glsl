#version 150 core

in vec2 position;
in float age;
in float life;
in vec2 velocity;

out float Percent;
out vec2 Position;

void main() {
  Percent = 1 - age / life;
  Position = position;

  gl_PointSize = 5.0 * Percent;
  gl_Position = vec4(position, 0.0, 1.0);
}
