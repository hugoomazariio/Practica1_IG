#version 330 core
#define PI 3.1415926538
out vec4 outColor;

in vec3 pos;
in vec2 coordText;
in vec3 norm;

//Texturas (samplers)
uniform sampler2D colorTex;
uniform sampler2D emiTex;
uniform sampler2D specularTex;
uniform sampler2D auxiliarTex;

uniform uint timer; //Para la animación (control del shader); (unsigned integrer)
float time = float(timer);

// Propiedades del objeto

vec3 Ka; // Componente ambiental
vec3 Kd; // Componente difusa
vec3 Ks; // Componente especular
vec3 Ke; //Componente emisiva

vec3 N; //Normal
float alpha = 80.0; // Exponente de brillo


// Propiedades de la luz
vec3 Ia = vec3(0.2); // Intensidad ambiental de la luz
vec3 Id = vec3(0.5); // Intensidad de la luz difusa
vec3 Is = vec3 (5.0); //Intensidad de luz especular
vec3 lpos = vec3(0.0, 2.0, 0.0); // Posición de la luz

// Función de sombreado
vec3 shade();

void main()
{
	Ke = texture(emiTex, coordText).rgb;
	Ka = texture(auxiliarTex, vec2 (coordText.s/2.0 + sin(PI*time/1000.0)/2.0, coordText.t/2.0 + sin(PI*time/1000.0)/2.0)).rgb; //s -> eje X; t -> eje Y
	Kd = texture(colorTex, coordText).rgb; 
	Ks = texture(specularTex, coordText).rgb;
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
	vec3 diffuse = Id * Kd * dot(L,N); // Dot product entre la Normal y la Luz
	c += clamp(diffuse, 0.0, 1.0); // Sumo la difusa (clamp me controla para que no haya un número negativo ni mayor que 1)
	
	// Especular Phong
	vec3 V = normalize (-pos); // Vector de la cámara - punto del objeto
	vec3 R  = normalize(reflect(-L, N)); // Quiero que me reflejes el vector de la luz respecto a la normal
										 // Es -L porque invierto el sentido de la dirección (en caso de no ser negativo, se verá la luz en el lado contrario)
	// Calculo el ángulo entre R y V
	float factor = clamp(dot(R,V), 0.0,1.0);
	
	vec3 specular = Is * Ks * pow (factor, alpha); // Pow es a elevado a d
	c +=  clamp(specular, 0.0, 1.0); 

	//Emisivas
	c += Ke;

	return c;
}
