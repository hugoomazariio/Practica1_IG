#version 330 core

in vec3 inPos;	
uniform mat4 modelViewProj; // Ahora soy más eficiente, y veo que la librería me ha hecho esa multiplicación

out vec3 color; // Variable variante
void main()
{
// Trabajo con el ID de los vértices
	if(mod(gl_VertexID, 4.0) == 0.0) // Divido entre 4 = Resto 0 -> cambia color del triángulo
		color = vec3(1.0, 0.0, 0.0);
	else if(mod(gl_VertexID, 4.0) == 1.0) 
		color = vec3(0.0, 1.0, 0.0);
	else if(mod(gl_VertexID, 4.0) == 2.0)
		color = vec3(0.0, 0.0, 1.0);
	else if(mod(gl_VertexID, 4.0) == 3.0)
		color = vec3(1.0, 1.0, 0.0);


	gl_Position = modelViewProj*vec4(inPos, 1.0); 
}
