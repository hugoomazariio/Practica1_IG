#version 330 core

in vec3 inPos;
in vec3 inColor; // Ahora tengo ya el color de entrada
uniform mat4 modelViewProj; // Ahora soy más eficiente, y veo que la librería me ha hecho esa multiplicación

out vec3 color; // Variable variante
void main()
{

	color = inColor; // Como es un vec3, lo asigno a inColor (ya tení antes esta variable)
	gl_Position = modelViewProj*vec4(inPos, 1.0); 
}
