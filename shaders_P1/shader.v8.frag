#version 330 core

out vec4 outColor;
in vec3 norm;


void main()
{
	//outColor = vec4((norm.x+1)/2.0,(norm.y+1)/2.0, (norm.z+1)/2.0); 
	outColor = vec4(1.0, 0.0, 0.0, 1.0);

}
