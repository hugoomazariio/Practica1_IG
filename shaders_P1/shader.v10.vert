#version 330 core

in vec3 inPos;	
in vec2 inTexCoord;
in vec3 inNormal; // Lo primero que necesito son las normales.

uniform mat4 normal;
uniform mat4 modelView;
uniform mat4 modelViewProj;

out vec2 coordTex;
out vec3 pos; 
out vec3 norm;

void main()
{
	norm = (normal * vec4(inNormal, 1.0)).xyz; // La normal que pasamos está interpolada, y no tiene mod 1, no hago ese normalize de antes y me lo ahorro.
	pos = (modelView * vec4(inPos, 1.0)).xyz; 
	coordTex = inTexCoord;
	gl_Position =  modelViewProj * vec4 (inPos,1.0);
}

