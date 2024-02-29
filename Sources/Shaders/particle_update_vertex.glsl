#version 150 core

in vec2 position;
in float age;
in float life;
in vec2 velocity;

uniform float deltaTime;
uniform sampler2D rgNoise;
// uniform vec2 u_Gravity;
uniform vec2 origin;
uniform float minTheta;
uniform float maxTheta;
uniform float minSpeed;
uniform float maxSpeed;

out vec2 Position;
out float Age;
out float Life;
out vec2 Velocity;

void main() {
	if (age >= life) {
		ivec2 noise_coord = ivec2(gl_VertexID % 512, gl_VertexID / 512);
		vec2 rand = texelFetch(rgNoise, noise_coord, 0).rg;
		float theta = minTheta + rand.r*(maxTheta - minTheta);

		float x = cos(theta);
		float y = sin(theta);

		Position = origin;
		Age = 0.0;
		Life = life;
		Velocity = vec2(x, y) * (minSpeed + rand.g * (maxSpeed - minSpeed));
	} else {
		Position = position + velocity * deltaTime;
		Age = age + deltaTime;
		Life = life;
		Velocity = velocity; // + u_Gravity * u_TimeDelta;
	}
}
