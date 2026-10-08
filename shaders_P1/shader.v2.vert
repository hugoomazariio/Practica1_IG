#version 330 core

in vec3 inPos;	
uniform mat4 proj; // Hay que usar este nombre en concreto porque así está declarado en esta práctica
uniform mat4 view; 
uniform mat4 model; 

void main()
{
	

	gl_Position = proj*view*vec4(inPos, 1.0) ; 
}
