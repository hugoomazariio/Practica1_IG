#version 330 core

out vec4 outColor;
in vec3 norm;


void main()
{
	outColor = vec4(norm,1.0); // Caras en negro porque en OpenGL los valores van de 0 a 1, al estar en negro están en negativo
							   // Es decir, antes apunta a -1, pero el programa apunta automáticamente a 0, porque lo intenta pintar.
	

}
