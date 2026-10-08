#version 330 core

out vec4 outColor;


void main()
{
	if (mod(gl_PrimitiveID, 4.0) == 0.0){
		outColor = vec4(0.0, 0.3, 1.0, 1.0);    
	} else if (mod(gl_PrimitiveID, 4.0) == 1.0){
		outColor = vec4(0.1, 1.0, 0.0, 1.0);
	}else if (mod(gl_PrimitiveID, 4.0) == 2.0){
		outColor = vec4(1.0, 0.0, 1.0, 1.0);
	}else if (mod(gl_PrimitiveID, 4.0) == 3.0){
		outColor = vec4(0.0, 0.0, 1.0, 1.0);

	}

}
