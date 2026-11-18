#version 330 core

in vec3 inPos;	
in vec3 inNormal;
in vec2 inTexCoord;
in vec3 inTangent;

uniform mat4 normal;
uniform mat4 modelView;
uniform mat4 modelViewProj;
uniform mat4 model;

out vec3 pos;
out vec3 norm;
out vec2 coordTex;
out vec3 tang;
out vec3 worldPos;

void main()
{
	norm = (normal * vec4(inNormal, 0.0f)).xyz;
	pos = (modelView * vec4(inPos, 1.0)).xyz;
	coordTex = inTexCoord;
	tang = (modelView * vec4 (inTangent, 0.0)).xyz;
	worldPos = (model * vec4(inPos, 1.0)).xyz;

	gl_Position =  modelViewProj * vec4 (inPos,1.0);
}