#version 330 core

out vec4 FragColor;

// (todo) 02.5: Add Color input variable here
in vec3 Color;
in float age;
in float duration;


void main()
{
	// (todo) 02.3: Compute alpha using the built-in variable gl_PointCoord

	float intensity = (1.0 - length (gl_PointCoord * 2 - 1));
	FragColor = vec4(Color, intensity);
}