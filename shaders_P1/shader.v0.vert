#version 330 core

in vec3 inPos;	
in vec3 inColor;
in vec3 inNormal;

uniform mat4 normal;
uniform mat4 modelView;
uniform mat4 modelViewProj;

out vec3 color;

// Propiedades del objeto
vec3 Ka; // Componente ambiental
vec3 Kd; // Componente difusa difusa
vec3 Ks = vec3(1.0); // Componente especular ((1, 1, 1), con ningún tipo de color)

vec3 pos; 
vec3 N; 
float alpha = 5.0; // Exponente de brillo

// Propiedades de la luz
vec3 Ia = vec3(0.2); // Intensidad de luz ambiental 
vec3 Id = vec3(0.8); // Intensidad de luz difusa
vec3 Is = vec3 (1.0); // Intensidad de luz especular
vec3 lpos = vec3(0.0, 2.0, 0.0); // Posición de la luz

// Función de sombreado
vec3 shade();

void main()
{
	Ka = inColor; // Asigno el color del cubo a Ka
	Kd = inColor; // Asigno el color del cubo a Kd
	color = shade();
	N = normalize((normal * vec4(inNormal, 1.0)).xyz);
	pos = (modelView * vec4(inPos, 1.0)).xyz; // Transformo la posicióm del vértice al de la cámara. 

	color = shade(); // Llamo al método para que le de la iluminación al cubo
	gl_Position =  modelViewProj * vec4 (inPos,1.0);
}

vec3 shade()
{
	// Ambiental
	vec3 c = vec3(0.0); 
	c  = Ia * Ka; // Con esto sale el color del objeto.

	// Difusa
	vec3 L = normalize(lpos - pos); // Vector de la luz normalizado para ese punto.
	vec3 diffuse = Id * Kd * dot(L,N); // Dot product entre la Normal y la Luz
	c += clamp(diffuse, 0.0, 1.0); // Sumo la difusa (clamp me controla para que no haya un número negativo)
	
	// Especular
	vec3 V = normalize (-pos); // Vector de la cámara - punto del objeto
	vec3 R  = normalize(reflect(-L, N)); // Quiero que me reflejes el vector de la luz respecto a la normal
										// Es -L porque invierto el sentido de la dirección (en caso de no ser negativo, se verá la luz en el lado contrario)
	// Calculo el ángulo entre R y V
	float factor = clamp(dot(R,V), 0.0,1.0);
	
	vec3 specular = Is * Ks * pow (factor, alpha); // Pow es a elevado a d
	c +=  clamp(specular, 0.0, 1.0); 
	return c;
}
