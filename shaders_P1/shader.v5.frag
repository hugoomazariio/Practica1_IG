#version 330 core
#define M_PI 3.1415926538

in vec3 pos;
in vec3 norm;
in vec2 coordTex;

uniform sampler2D colorTex;
uniform sampler2D emiTex;
uniform sampler2D normalTex;
uniform sampler2D specularTex;
uniform sampler2D auxiliarTex;

uniform mat4 view; // Matriz de vista para pasar las luces a coordenadas de cámara (cuando movemos la cámara, las luces "hacen el movimiento contrario")

out vec4 outColor;

// Propiedades del objeto
vec3 Ka; // Componente ambiental
vec3 Kd; // Componente difusa
vec3 Ks; // Componente especular
vec3 Ke; // Componente emisiva

vec3 N; // Normal
float alpha = 80.0; // Exponente de brillo

// Propiedades de la luz
vec3 Ia = vec3(0.2, 0.05, 0.2); // Intensidad de luz ambiental
vec3 Id = vec3(0.1, 0.0, 0.7);// Intensidad de luz difusa
vec3 Is = vec3(1.0, 0.5, 0.8); // Intensidad de luz especular

vec3 lPos = vec3 (0.0, 2.0, 0.0); // Posición de la luz

// Función auxiliar para el cálculo de Phong (luz difusa y especular)
vec3 phong (vec3 L, vec3 lD, vec3 lS);

// Función de sombreado
vec3 shade();


void main()
{
	Kd = texture(colorTex, coordTex).rgb;
	Ka = Kd;
	Ks = texture(specularTex, coordTex).rgb;
	Ke = texture(emiTex, coordTex).rgb;

	N = normalize(norm);

	outColor = vec4 (shade(), 1.0);
}

// Función auxiliar que calcula con una luz proveniente de la dirección L con intensidad difusa lD e especular lS
vec3 phong (vec3 L, vec3 lD, vec3 lS) {

	vec3 c = vec3 (0.0f);

	// LUZ DIFUSA
	float dotNL = max (dot(N, L), 0.0); //Nos quedamos con el producto solo si es positivo
	vec3 diffuse = lD * Kd * dotNL;

	c += diffuse;

	// LUZ ESPECULAR
	if (dotNL > 0.0) { // Solo se hará el cálculo especular si la luz llega al fragmento (producto > 0)
		vec3 V = normalize(-pos);
		vec3 R = normalize(reflect(-L, N)); 

		float factor = max(dot(V, R), 0.0); // Ángulo entre V y R

		vec3 specular = lS * Ks * pow(factor, alpha);

		c += specular;
	}

	return c;
}

vec3 shade() {

	vec3 c = vec3(0.0); 

	// LUZ AMBIENTAL (Solo lo calculamos una vez, dado que aplica a toda la escena por igual)
	c += Ia * Ka;

	// LUZ DIRECCIONAL
	vec3 direcLuzDireccional = normalize(vec3 (-1.0, -1.0, -1.0)); //Es la misma para cualquier objeto
	direcLuzDireccional = (view * vec4(direcLuzDireccional, 0.0)).xyz; //Lo pasamos a coordenadas de cámara
	c += phong (-direcLuzDireccional, vec3(0.0, 0.0, 0.4), vec3(0.0, 0.0, 0.6)); // L = -D

	// LUZ PUNTUAL 
	vec3 posLuzPuntual = vec3 (1.0, 1.0, 0.0);
	posLuzPuntual = (view * vec4(posLuzPuntual, 1.0)).xyz; //Lo pasamos a coordenadas de cámara

	vec3 LP = (posLuzPuntual - pos); //Vector de luz

	float dP = length(LP); //Distancia entre la luz y el punto 
	float atenuacionP = 1.0 / (1.0 + 0.2 * dP + 0.1 * dP * dP); 

	c += phong (normalize(LP), vec3(0.3, 0.0, 0.0) * atenuacionP, vec3(0.3, 0.1, 0.2) * atenuacionP);

	//LUZ FOCAL
	vec3 posLuzFocal = vec3 (0.0, 1.0, 0.0);
	posLuzFocal = (view * vec4(posLuzFocal, 1.0)).xyz;

	vec3 direcLuzFocal = vec3 (0.0, -1.0, 0.0);
	direcLuzFocal = (view * vec4(direcLuzFocal, 0.0)).xyz;

	vec3 LF = (posLuzFocal - pos); 

	float cosAngulo = dot(-(normalize(LF)), direcLuzFocal); // Ángulo entre el foco y el fragmento
	float corte = 25; // Ángulo de corte
	float penumbra = 30; // Ángulo de penumbra
	

	float foco = smoothstep(cos(radians(penumbra)), cos(radians(corte)), cosAngulo); //Realizamos una interpolación para suavizar la luz

	c += phong(normalize(LF), vec3(0.4, 0.5, 0.0) * foco, vec3(1.0) * foco);

	// COMPONENTE EMISIVA (solo una vez)
	c += Ke;

	return c;
}