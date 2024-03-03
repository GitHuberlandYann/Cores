#version 150 core

in float Percent;
in vec2 Position;
in float Speed;

uniform vec4 birthColor;
uniform vec4 deathColor;
uniform vec3 speedColor;

out vec4 outColor;

void main() {
	/* outColor = vec4(Percent, Speed, 1 - Percent, Percent);
		percent = 1 = birth = (1, 0, 0, 1);
		percent = 0 = death = (0, 0, 1, 0);
		speed = 1 = (0, 1, 0, 0);*/
	outColor = mix(deathColor, birthColor, Percent);
	outColor = mix(outColor, vec4(speedColor, outColor.a), Speed);
}
