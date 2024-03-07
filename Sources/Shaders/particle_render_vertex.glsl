#version 150 core

in vec2 position;
in float age;
in float life;
in vec2 velocity;

uniform ivec2 winPos;
uniform ivec2 winSize;
uniform float minSpeed;
uniform float maxSpeed;
uniform float birthSize;
uniform float deathSize;
uniform float lifeSpan;
uniform float lifeRange;

out float Percent;
out vec2 Position;
out float Speed;

void main() {
	Percent = 1 - age / (lifeSpan + life * lifeRange);
	Position = position;
	Speed = (clamp(length(velocity), minSpeed, maxSpeed) - minSpeed) / (maxSpeed - minSpeed);

	// gl_PointSize = 5.0 * Percent;
	gl_PointSize = mix(deathSize, birthSize, Percent);

	gl_Position = vec4(((position.x - winPos.x) / winSize.x) * 2 - 1, ((position.y - winPos.y) / winSize.y) * -2 + 1, Percent, 1.0);
}
