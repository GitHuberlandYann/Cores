#version 150 core

in float Percent;
in vec2 Position;

out vec4 outColor;

void main() {
//   outColor = vec4(Percent, 0, 1 - Percent, Percent);
  outColor = vec4((Position.x + 1) / 2, (Position.y + 1) / 2, 1 - Percent, Percent);
}
