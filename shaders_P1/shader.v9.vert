#version 330 core

in vec3 inPos;
in vec3 inNormal; // Meto las coords de normales

uniform mat4 modelViewProj; // Ahora soy más eficiente, y veo que la librería me ha hecho esa multiplicación
uniform mat4 modelView;

out vec3 norm; 

void main()
{
	// Ahora lo que tengo que hacer es la traspuesta de mi inversa, porque la normal no se queda perpendicular
	norm = (transpose(inverse(modelView))* vec4(inNormal, 0)).xyz; // Cuando opero con una dirección, opero con 0, coords homogéneas!!! 
	gl_Position = modelViewProj*vec4(inPos, 1.0); 

	// Aquí las normales están en coordenadas de vista
}
