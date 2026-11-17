#version 330 core

out vec4 outColor;

in vec3 pos;
in vec3 color;
in vec3 norm;


// Propiedades del objeto
vec3 Ka; // Componente ambiental
vec3 Kd; // Componente difusa
vec3 Ks = vec3(1.0); // Componente especular
vec3 N; //Normal
float alpha = 50.0; // Exponente de brillo


// Propiedades de la luz
vec3 Ia = vec3(0.2); // Intensidad ambiental de la luz  
vec3 Id = vec3(0.5); // Intensidad difusa de la luz 
vec3 Is = vec3 (0.3, 0.3, 0.0); //Intensidad especular de luz 
vec3 lpos = vec3(0.0, 2.0, 0.0); // Posición de la luz

// Función de sombreado
vec3 shade();

void main()
{
	Ka = color; 
	Kd = color; 
	N = normalize(norm);
	outColor = vec4(shade(), 1.0);   
}

vec3 shade()
{
	// Ambiental
	vec3 c = vec3(0.0); 
	c  = Ia * Ka; // Con esto sale el color del objeto.

	// Difusa
	vec3 L = normalize(lpos - pos); // Vector de la luz normalizado para ese punto.
	vec3 diffuse = Id * Kd * dot(L,N); // Dot product entre la Normal y el vector de la Luz
	c += clamp(diffuse, 0.0, 1.0); // Sumo la difusa (clamp me controla para que no haya un número negativo ni superior a 1)
	
	// Especular Phong
	vec3 V = normalize (-pos); // Vector de la cámara (0, 0, 0) - punto del objeto
	vec3 R  = normalize(reflect(-L, N)); // Quiero que me reflejes el vector de la luz respecto a la normal
										 // Es -L porque invierto el sentido de la dirección (en caso de no ser negativo, se verá la luz en el lado contrario)
	// Calculo el ángulo entre R y V
	float factor = clamp(dot(R,V), 0.0,1.0);
	
	vec3 specular = Is * Ks * pow (factor, alpha); // Pow es a elevado a d
	c +=  clamp(specular, 0.0, 1.0); 
	return c;
}
