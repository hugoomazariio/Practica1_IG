#version 330 core
#define PI 31.1415926538
out vec4 outColor;

in vec3 pos;
in vec2 coordTex;
in vec3 norm;


// Texturas (samplers)

uniform sampler2D colorTex;
uniform sampler2D emiTex;
uniform sampler2D specularTex;
uniform sampler2D auxiliarTex;

uniform uint timer; // Timer para las animaciones, es un unsigned int

float time = float(timer); // Si no hago esta conversion a float, no podría hacer una division entera luego

// Propiedades del objeto

vec3 Ka; // Componente ambiental del objeto
vec3 Kd; // Iluminación difusa
vec3 Ks; // Esto hace que el brillo no tenga ningún tipo de color
vec3 Ke; // Componente emisiva

vec3 N; 
float alpha = 80.0; // Ángulo

// Propiedades de la luz

vec3 Ia = vec3(0.2); // Intensidad ambiental de la luz
vec3 Id = vec3(0.5); // Intensidad de la luz difusa
vec3 Is = vec3 (0.3, 0.3, 0.0);
vec3 lpos = vec3(0.0, 2.0, 0.0); // Posición de la luz

// Función de sombreado
vec3 shade();

void main()
{
	Ke = texture(emiTex, coordTex).rgb; 
	Ka = texture(auxiliarTex, vec2(coordTex.s/2.0 + sin(PI * time/1000.0)/2.0, // CoordTex.s es la primera coordenada de la textura (coord x)
							coordTex.t/2.0 + sin(PI * time/1000.0)/2.0)).rgb; // CoordTex.t es la segunda coordenada de la textura (coord y)
							// Me estoy desplazando en diagonal en la textura
	Kd = texture(colorTex, coordTex).rgb;
	Ks = texture(specularTex, coordTex).rgb;
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
	c += clamp(diffuse, 0.0, 1.0); // Sumo la difusa (clamp me controla para que no haya un número negativo)
	
	// Especular Phong

	vec3 V = normalize (-pos); // Vector de la cámara - punto del objeto
	vec3 R  = normalize(reflect(-L, N)); // Quiero que me reflejes el vector de la luz respecto a la normal
										// Es -L porque invierto el sentido de la dirección (en caso de no ser negativo, se verá la luz en el lado contrario)
	// Calculo el ángulo entre R y V
	float factor = clamp(dot(R,V), 0.0,1.0);
	
	vec3 specular = Is * Ks * pow (factor, alpha); // Pow es a elevado a d
	c +=  clamp(specular, 0.0, 1.0); 
	c+=Ke;
	return c;
}
