#version 330 core

in vec3 inPos;
in vec3 inNormal; // Meto las coords de normales
uniform mat4 modelViewProj; // Ahora soy más eficiente, y veo que la librería me ha hecho esa multiplicación

out vec3 norm; 

void main()
{

	norm = inNormal;
	gl_Position = modelViewProj*vec4(inPos, 1.0); 
}
