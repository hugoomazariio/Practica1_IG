#version 330 core

in vec3 inPos;	
#define M_PI 3.14159265

void main()
{
	//model_view matrix
	mat4 modelView = mat4(1.0); // Inicializo la matriz como matriz identidad

	// Ahora posiciono la cámara en el eje z -3 y luego voy a invertir su dirección
	modelView[3].z = -3.0; // Esto es un acceso de la matriz, es una forma de acceder a la matriz, columna 4 fila 4 (el array empieza en 0)

	// Con esto tendría suficiente para una matriz de vista, ahora hago una matriz de proyección
	mat4 proj = mat4(1.0);

	// Construyo f, near y far
	float f = 1.0 / tan(M_PI/6.0);
	float far = 10.0; // Plano lejano
	float near = 0.1; // Plano cercano

	// Cojo elemento a elemento de la matriz de proyección y voy a ir tocandolos uno a uno

	proj[0].x = f; // Primera columna del elemento de la matriz
	proj[1].y = f; // El uno a uno de la diagonal de la matriz
	proj[2].z = (far + near)/ (near - far);
	proj[2].w  =-1.0;
	proj[3].z  = (2.0*far*near)/(near-far);
	proj[3].w = 0.0;

	// Por último tengo que pasar la posición, tendré que pasar lo que me llega de entrada (inPos)
	gl_Position = proj*modelView*vec4(inPos, 1.0) ; // Construyo un vec4 (en coords homogeneas no puedo meter vec3) y meto un 1 (a puntos meto un 0 y a vectores meto un 1)
}
