#version 330 core
#define M_PI 3.1415926538

in vec3 pos;
in vec3 norm;
in vec2 coordTex;
in vec3 tang;
in vec3 worldPos; //Posición del objeto en el mundo

uniform sampler2D colorTex;
uniform sampler2D emiTex;
uniform sampler2D normalTex;
uniform sampler2D specularTex;
uniform sampler2D auxiliarTex;

uniform uint timer; // Timer para el control del shader
float time = float(timer);

uniform mat4 view; // Matriz de vista para pasar las luces a coordenadas de cámara (para que no se muevan con la cámara)

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
vec3 Id = vec3(0.1, 0.0, 0.7); // Intensidad de luz difusa
vec3 Is = vec3(1.0, 0.5, 0.8); // Intensidad de luz especular

vec3 lPos = vec3 (0.0, 2.0, 0.0); // Posición de la luz

// Función auxiliar para el cálculo de Phong (luz difusa y especular)
vec3 phong (vec3 L, vec3 lD, vec3 lS);

//Funciones auxiliares para value noise
float hash(vec3 p);
float valueNoise(vec3 p);
float fbm(vec3 p);

// Función de sombreado
vec3 shade();

void main()
{
	// ------- Texturas ------- 
	Ka = texture(auxiliarTex, coordTex).rgb;
	Kd = texture(colorTex, coordTex).rgb;
	Ks = texture(specularTex, coordTex).rgb;
	Ke = texture(emiTex, coordTex).rgb;

	// ------- Normal Mapping ------- 
	 vec3 T = normalize(tang); // Vector tangente
	 N = normalize (norm); // Vector normal
	 T = normalize(T - dot(T, N) * N); // Gram-Schmidt para asegurarnos que T y N son perpendiculares
	 vec3 B = cross(N, T); // Vector bitangente

	 mat3 TBN = mat3(T, B, N);

	 vec3 normTex = texture(normalTex, coordTex).rgb * 2.0 - 1.0; // Pasamos de [0, 1] a [-1, 1]
	 N = normalize (TBN * normTex);

	 // ------- Niebla ------- 
	  float fogHeight = 0.1;
	  float fogThickness = 0.5;
	  float heightFactor = 1.0 - smoothstep (fogHeight - fogThickness, fogHeight, worldPos.y);

	  vec3 wind = vec3(0.3, 0.0, 0.1) * (time/5000);
	  float maxDensity = 0.8;
	  float density = maxDensity * heightFactor * fbm(worldPos * 1.5 + wind);

	  float dist = length(pos);
	  float f = exp(-density * dist);

	  vec3 fogColor = vec3(0.4, 0.35, 0.5); 
	  vec3 color = shade(); // Guardamos los cálculos de la iluminación en una variable

	  //outColor = vec4(vec3(heightFactor), 1.0); //DEPURACIÓN -- DONDE SE CORTA LA NIEBLA
	  outColor = vec4(mix(fogColor, color, f), 1.0);
}

// --------------- Iluminación --------------- 
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
	vec3 direcDirectionLight = normalize(vec3 (-1.0, -1.0, -1.0)); //Es la misma para cualquier objeto
	direcDirectionLight = (view * vec4(direcDirectionLight, 0.0)).xyz; //Lo pasamos a coordenadas de cámara
	c += phong (-direcDirectionLight, vec3(0.0, 0.0, 0.4), vec3(0.0, 0.0, 0.6)); // L = -D

	// LUZ PUNTUAL (luciérnaga)
	float angFirefly = mod(time * 0.001f, 2.0 * M_PI); //Ángulo de movimiento de la luciernaga
	vec3 centre = vec3(0.0, 0.0, 0.0); //Centro de la órbita
	float radius = 0.5f; //Radio de la óribita de la luciernaga

	float flickVel = time / 5000.0; //Dividimos el tiempo para que el parpadeo no vaya demasiado rápido
	float wave = 0.5 + 0.5 * sin(flickVel * M_PI); // Onda que va de 0 a 1
	float fireflyFlickering = 0.2 + 0.8 * pow(wave, 2.0); //Parpadeo de la luciernaga

	vec3 fireflyColor = vec3(0.2, 0.9, 0.3) * fireflyFlickering;

	//Calculamos la posición de la luz, que tiene que ser el mismo que el de la luciérnaga
	vec3 posLightFirefly = vec3 (centre.x + cos(angFirefly) * radius,
								centre.y + 1.0 + sin(angFirefly * 3.0) * 0.3,
								centre.z + sin(angFirefly) * radius);

	vec3 posLFireflyCam = (view * vec4(posLightFirefly, 1.0)).xyz; //Lo pasamos a coordenadas de cámara
	vec3 LL = (posLFireflyCam - pos); //Vector de luz

	float dL = length(LL); //Distancia entre la luz y el punto (la luciernaga)
	float atenuationL = 1.0 / (1.0 + 0.7 * dL + 1.8 * dL * dL); 

	c += phong (normalize(LL), fireflyColor * atenuationL, fireflyColor * atenuationL);

	// LUZ FOCAL
	vec3 posFocalLight = vec3 (0.0, 1.0, 0.0);
	posFocalLight = (view * vec4(posFocalLight, 1.0)).xyz;

	vec3 direcFocalLight = vec3 (0.0, -1.0, 0.0);
	direcFocalLight = (view * vec4(direcFocalLight, 0.0)).xyz;

	vec3 LF = (posFocalLight - pos); 

	float cosAngle = dot(-(normalize(LF)), direcFocalLight); // Ángulo entre el foco y el fragmento
	float corte = 25; // Ángulo de corte
	float penumbra = 30; // Ángulo de penumbra
	
	float foco = smoothstep(cos(radians(penumbra)), cos(radians(corte)), cosAngle); //Realizamos una interpolación para suavizar la luz

	c += phong(normalize(LF), vec3(0.4, 0.5, 0.0) * foco, vec3(1.0) * foco);

	// COMPONENTE EMISIVA (solo una vez)
	c += Ke;

	return c;
}

// --------------- Ruido --------------- 
float hash(vec3 p) {
	p = fract(p * 0.3183099 + 0.1);
	p *= 17.0;
	return fract(p.x * p.y * p.z * (p.x + p.y + p.z));
}

float valueNoise(vec3 p) {
	vec3 i = floor(p);
	vec3 f = fract(p);
	f = f * f * (3.0 - 2.0 * f);

	return mix(mix(mix(hash(i + vec3(0,0,0)), hash(i + vec3(1,0,0)), f.x),
	               mix(hash(i + vec3(0,1,0)), hash(i + vec3(1,1,0)), f.x), f.y),
	           mix(mix(hash(i + vec3(0,0,1)), hash(i + vec3(1,0,1)), f.x),
	               mix(hash(i + vec3(0,1,1)), hash(i + vec3(1,1,1)), f.x), f.y), f.z);

}

float fbm (vec3 p) {
	float value = 0.0;
	float amplitude = 0.5;

	for (int i = 0; i < 4; i++) {
		value += amplitude * valueNoise(p);
		p *= 2.0;
		amplitude *= 0.5;
	}

	return value;
}