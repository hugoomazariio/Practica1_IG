#version 330 core

in vec3 inPos;	
uniform mat4 modelViewProj; // Ahora soy más eficiente, y veo que la librería me ha hecho esa multiplicación

void main()
{
	

	gl_Position = modelViewProj*vec4(inPos, 1.0) ; 
}
