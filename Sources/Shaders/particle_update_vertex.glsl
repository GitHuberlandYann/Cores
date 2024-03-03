#version 150 core

in vec2 position;
in float age;
in float life;
in vec2 velocity;

uniform float deltaTime;
uniform sampler2D rgNoise;
uniform vec2 origin;
uniform vec3 gravity[10];
uniform int polarity[10];
uniform float minTheta;
uniform float maxTheta;
uniform float minSpeed;
uniform float maxSpeed;
uniform float terminalVelocity;

out vec2 Position;
out float Age;
out float Life;
out vec2 Velocity;

void main() {
	if (age >= life) {
		ivec2 noise_coord = ivec2(gl_VertexID & 0x1FF, gl_VertexID >> 8);
		vec2 rand = texelFetch(rgNoise, noise_coord, 0).rg;
		float theta = minTheta + rand.r * (maxTheta - minTheta);

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
		vec2 f_gravity = vec2(0, 0);
		vec2 n_velocity = velocity;
		for (int i = 0; i < 10; ++i) {
			if (gravity[i].x < 999999 && gravity[i].z > 0) {
				// F = G * (Ma * Mb) / d²
				// float dist = max(100.0f, distance(position, gravity)); // 0.2
				float dist = distance(position, vec2(gravity[i]));
				f_gravity = ((polarity[i] == 0) ? 1 : -1) * normalize(vec2(gravity[i]) - position) * pow(10, gravity[i].z) / (dist * dist);
				n_velocity += f_gravity * deltaTime;
			}
		}
		if (length(n_velocity) > terminalVelocity) {
			n_velocity = velocity;
			// n_velocity = terminalVelocity * normalize(n_velocity);
		}
		Velocity = n_velocity;
	}
}
