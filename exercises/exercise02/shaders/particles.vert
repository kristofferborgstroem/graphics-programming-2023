#version 330 core

layout (location = 0) in vec2 ParticlePosition;
layout (location = 1) in float ParticleSize;
layout (location = 2) in float ParticleBirth;
layout (location = 3) in float ParticleDuration;
layout (location = 4) in vec3 ParticleColor;
layout (location = 5) in vec2 ParticleVelocity;
layout (location = 6) in vec2 gravity;
// (todo) 02.X: Add more vertex attributes


// (todo) 02.5: Add Color output variable here

out vec3 Color;
out float age;
out float duration;


// (todo) 02.X: Add uniforms
uniform float CurrentTime;

void main()
{

	Color = ParticleColor;
	age = CurrentTime - ParticleBirth;
	duration = ParticleDuration;
	gl_PointSize = (1.0 - ((duration - age) / duration)) * 50.0;


	if (age < ParticleDuration) {
		gl_Position = vec4(ParticlePosition + ParticleVelocity * age + 0.5 * gravity * age * age, 0.0, 1.0);
	} else {
		gl_Position = vec4(1000.0, 1000.0, 0.0, 1.0);
	}
}
