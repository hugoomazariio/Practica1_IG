#version 330 core

in vec3 inPos;
in vec2 inTexCoord; // Para pintar una textura, primero necesito las coordenadas de la textura

uniform mat4 modelViewProj; // Ahora soy más eficiente, y veo que la librería me ha hecho esa multiplicación

out vec2 texCoord; 

void main()
{
	texCoord = inTexCoord;
	gl_Position = modelViewProj*vec4(inPos, 1.0); 
}
