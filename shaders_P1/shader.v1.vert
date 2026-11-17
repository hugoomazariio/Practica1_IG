#version 330 core

in vec3 inPos;	
in vec3 inColor;
in vec3 inNormal;

uniform mat4 normal;
uniform mat4 modelView;
uniform mat4 modelViewProj;

out vec3 color;
out vec3 pos; 
out vec3 norm;

void main()
{
	norm = (normal * vec4(inNormal, 1.0)).xyz; // La normal que pasamos está interpolada, y no tiene mod 1, no hago ese normalize de antes y me lo ahorro
	pos = (modelView * vec4(inPos, 1.0)).xyz; 
	color = inColor;
	gl_Position =  modelViewProj * vec4 (inPos,1.0);
}

