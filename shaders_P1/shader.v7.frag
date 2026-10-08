#version 330 core

out vec4 outColor;
in vec2 texCoord;

uniform sampler2D colorTex;

void main()
{
	//outColor = texture(colorTex, texCoord); // Solo la textura
	outColor = vec4(texture(colorTex, texCoord).rg,0.5,1.0); // Si pongo un 0 en el 0.5 pasa a ser completamente amarillo.
	// Esta línea de arriba lo que hace es darle color 

	outColor = vec4(texCoord.st, 0.0,1.0); // el st sirve para visualizar las coords de textura en lugar de las de imagen

}
