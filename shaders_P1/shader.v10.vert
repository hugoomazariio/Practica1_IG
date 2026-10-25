#version 330 core

in vec3 inPos;	
in vec2 inTexCoord;

uniform mat4 normal;
uniform mat4 modelView;
uniform mat4 modelViewProj;

out vec2 coordTex;

void main()
{
	coordTex = inTexCoord;
	gl_Position =  modelViewProj * vec4 (inPos,1.0);
}