#version 330 core

out vec4 outColor;

//Entradas desde el Vertex Shader
in vec3 pos;
in vec2 coordTex;

// Textura difusa principal
uniform sampler2D colorTex;

void main()
{
	outColor = texture(colorTex, coordTex);
}