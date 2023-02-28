#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

// (todo) 03.X: Add the out variables here
out vec3 Normal;
out vec3 VertexPos;

// (todo) 03.X: Add the uniforms here
uniform mat4 Model = mat4(1.0);
uniform mat4 ViewProjMatrix = mat4(1.0);


void main()
{
	Normal = (Model * vec4(VertexNormal, 0.0)).xyz;
	VertexPos = VertexPosition; //(ViewProjMatrix * Model * vec4(VertexPosition, 1.0)).xyz;
	gl_Position = ViewProjMatrix * Model * vec4(VertexPosition, 1.0);
}
