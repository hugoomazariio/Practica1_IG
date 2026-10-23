#include "BOX.h"
#include <IGL/IGlib.h>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cfloat>
#define M_PI 3.14159265

// Assimp library
#include <assimp/Importer.hpp>  // Esto sirve para la importación de ficheros
#include <assimp/scene.h> // Estructura de la escena
#include <assimp/postprocess.h> // Flags de procesado


// ---------------------- Variables ---------------------- 
//Idenficadores de los objetos de la escena
int objId = -1;

//creación de variables para los nuevos objetos a cargar en escena
int objeto1Id = -1;
int objeto2Id = -1;
int objeto3Id = -1;

// Variables assimp
const aiScene* sc; 

//Variables para movimiento de la luciérnaga
std::vector<int> fireflyIds;
float fireflyAngle = 0.0f;
float fireflyRadius = 1.5f;

glm::vec3 bikePos(-2.7f, -2.0f, -2.0f); //Posición de la moto para que la luciérnaga vuele a su alrededor


//Variables para movimiento de la cámara
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 4.0f); // Posición de la cámara en el espacio
glm::vec3 targetPos = glm::vec3(0.0f, 0.0f, -1.0f); // Hacia donde mira la cámara
glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f); //Vector hacia arriba (eje Y positivo)

float angHor = -90.0f; //Ángulo inicial de la cámara (horizontal). Comenzamos en -90 para que la cámara comience mirando hacia delante 
float angVert = 0.0f; //Ángulo inicial de la cámara (vertical)

//Variables para controlar el movimiento del ratón
float lastX = 400.0f;
float lastY = 300.0f;
bool firstMouse = true; // Nos ayuda a reducir saltos incómodos al mover la cámara
bool rightClicPressed; // Detecta si se está pulsando el clic derecho para el movimiento de cámara

glm::vec3 frontCam; //Vector que apunta delante de la cámara
glm::vec3 rightCam; //Vector que apunta a la derecha de la cámara
glm::vec3 upCam; //Vector que apunta arriba de la cámara

//Variables para zoom 
float fov = 60.0f; //Ángulo inicial de visión
float aspectRatio = 1.0f; //Proporción de la ventana (ancho-alto)

//Variables para seleccionar y escalar objetos
//Un modelo cargado puede tener varias mallas (una por material), así que se guardan todas juntas
struct Modelo {
	std::vector<int> ids; //IDs de las mallas (los que devuelve createObj)
	glm::vec3 posicion; //Dónde está el modelo en el mundo
	glm::vec3 escala; //Escala en cada eje
	glm::mat4 rotacion; // Rotación acumulado del modelo (matriz 4x4)
	float escalaInicial; //Escala con la que se cargó el modelo (para el reset con R)
	glm::vec3 posicionInicial; //Posición con la que se cargó el modelo (para el reset con R)
	glm::vec3 boundsMin, boundsMax; //Caja que rodea al modelo (en coordenadas del propio modelo), para detectar el clic
};
std::vector<Modelo> modelos; //Todos los modelos de la escena
int seleccionado = -1; //Índice del modelo seleccionado (-1 = ninguno)
bool modoEscalado = false; //Se activa con la tecla 1
bool modoRotacion = false; // Se activa con la tecla 2
bool modoTraslacion = false; // Se activa con la tecla 3
int indiceLuciernaga = -1; //Índice (en "modelos") de la luciérnaga, que se mueve sola en idleFunc
int ejeEscalado = 0; //Eje de escalado y de rotación: 0 = los tres ejes (en rotación, 0 = eje Y), 1 = X, 2 = Y, 3 = Z

glm::mat4 viewMat = glm::mat4(1.0f); //Copia de la matriz de vista (se necesita para saber qué objeto hay bajo el ratón)
glm::mat4 projMat = glm::mat4(1.0f); //Copia de la matriz de proyección
int anchoVentana = 500; //Tamaño de la ventana en píxeles
int altoVentana = 500;

////////////////////////////////////////////////
//Declaraci�n de CB
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);
void mouseMoveFunc(int x, int y);
void mousePassMoveFunc(int x, int y);
void mouseWheelFunc(int wheel, int dir, int x, int y);

//Métodos auxiliares
void updateCameraVectors(); //Para actualizar los vectores de la cámara según la entrada del teclado
glm::mat4 lookAtMatrix(glm::vec3 pos, glm::vec3 target, glm::vec3 up); //Para crear una matriz lookAt

void updateProjectionMatrix(); //Para actualizar los datos de la matriz de proyección (Zoom)

//Selección y escalado de objetos
glm::mat4 matrizModelo(const Modelo& m); //Matriz de modelo = traslación * escala
void actualizarModelo(const Modelo& m); //Envía la matriz de modelo a todas las mallas del modelo
void seleccionarObjeto(int mx, int my); //Selecciona el modelo que haya bajo el ratón
bool teclaEditor(unsigned char key); //Gestiona las teclas del editor (devuelve true si ha usado la tecla)
void escalarSeleccionado(float factor); //Multiplica la escala del modelo seleccionado
void rotarSeleccionado(float grados); //Gira el modelo seleccionado alrededor del eje elegido
void trasladarSeleccionado(float distancia); //Mueve el modelo seleccionado sobre el eje elegido
bool rayoCaja(glm::vec3 o, glm::vec3 d, glm::vec3 bmin, glm::vec3 bmax, float& t); //Intersección rayo-caja

///////////////////////////////////////////////////
// Carga de objetos
bool DoTheImportThing(const std::string& pFile, int &outObjId, float valorEscalado, glm::vec3 position);



// ---------------------- Método main ---------------------- 
int main(int argc, char** argv)
{
	std::locale::global(std::locale("spanish"));// acentos ;)
	if (!IGlib::init("../shaders_P1/shader.v9.vert", "../shaders_P1/shader.v9.frag")) 
		return -1;
	
	//Matriz de proyección
	updateProjectionMatrix();

	//Matriz de vista - Movimiento de la cámara (definimos 3 vectores perpendiculares, cada uno para cada eje)
	frontCam = glm::normalize(targetPos - cameraPos); // Vector desde la posición de la cámara hasta la posición a la que apunta (Eje z)
	rightCam = glm::normalize(glm::cross(frontCam, upVector)); //Vector hacia la derecha (eje X)
	upCam = glm::normalize(glm::cross(rightCam, frontCam)); //Vector hacia arriba (eje Y)

	updateCameraVectors();
	
	//se pasa el .obj de los objetos que se quieren cargar si no se encuenta devuelve -1
	if (!DoTheImportThing("../3D_assets/Bike/Bike.obj", objeto1Id, 0.5f, glm::vec3(-2.7f,-2.0f,-2.0f))) {
		return -1;
	}

	if (!DoTheImportThing("../3D_assets/Firefly/Firefly.obj", objeto2Id, 0.02f, glm::vec3(2.5f, -6.8f, -2.0f))) {
		return -1;
	}

	if (!DoTheImportThing("../3D_assets/Sword/Sword.obj", objeto3Id, 0.5f, glm::vec3(-1.5f, -2.0f, -2.0f))) {
		return -1;
	}

	//Creamos el objeto que vamos a visualizar
	// Cuando se crea un objeto en OpenGL se crea un ID, este ID hay que conservarlo para luego operaciones que se hagan 
	//objId = IGlib::createObj(cubeNTriangleIndex, cubeNVertex, cubeTriangleIndex, cubeVertexPos, cubeVertexColor, cubeVertexNormal, cubeVertexTexCoord, cubeVertexTangent); 
	// Cada vez que creemos un objeto, tenemos que guardar ese ID

	//glm::mat4 modelMat = glm::mat4(1.0f);
	//IGlib::setModelMat(objId, modelMat); // Te dice, dime el id y la matriz
	//Incluir texturas aqu�.
	//IGlib::addColorTex(objId, "../img/color.png");


	// CBs (Callbacks)
	// Cada vez que el sistema detecte que el usuario ha rescalado la pantalla, hecho click, etc. Recoge los eventos y nos avisa
	IGlib::setResizeCB(resizeFunc);
	IGlib::setIdleCB(idleFunc);
	IGlib::setKeyboardCB(keyboardFunc);
	IGlib::setMouseCB(mouseFunc);
	IGlib::setMouseMoveCB(mouseMoveFunc);
	IGlib::setMousePassiveMoveCB(mousePassMoveFunc);
	IGlib::setMouseWheelMoveCB(mouseWheelFunc);


	// Mainloop
	IGlib::mainLoop();
	IGlib::destroy();
	return 0;
}

// ---------------------- CB ---------------------- 
void resizeFunc(int width, int height)
{
	//AJUSTAR EL ASPECT RATIO MEDIANTE MATRIZ DE PROYECCIÓN
	if (height == 0)
		height = 1; //Para evitar divisiones entre 0

	aspectRatio = (float)width / (float)height; //Para guardar el ratio de la cámara (matriz de proyección)
	anchoVentana = width; //Guardamos el tamaño de la ventana para convertir el clic en un rayo
	altoVentana = height;
	updateProjectionMatrix();
}

void idleFunc()
{

	//std::cout << fireflyAngle << std::endl;
	//Movimiento de la luciérnaga
	fireflyAngle += 0.001f;

	glm::vec3 fireflyPos;

	fireflyPos.x = bikePos.x + cos(fireflyAngle) * fireflyRadius;
	fireflyPos.z = bikePos.z + sin(fireflyAngle) * fireflyRadius;
	

	// Pequeño movimiento vertical para simular vuelo
	fireflyPos.y = bikePos.y + 2.0f + sin(fireflyAngle * 3.0f) * 0.3f;

	//La posición la mueve la órbita, pero la escala la controla el editor (así se puede escalar la luciérnaga con el teclado)
	if (indiceLuciernaga >= 0) {
		modelos[indiceLuciernaga].posicion = fireflyPos;
		actualizarModelo(modelos[indiceLuciernaga]);
	}

	/*static float angle = 0.0f;

	angle = (angle < M_PI * 2.0) ? angle + 0.001 : 0.0f; // Si el ángulo es más de 60 grados, lo amplío, y sino reseteo 0.0. Hará que esté continuamente en rotación

	glm::mat4 modelMat = glm::translate(glm::mat4(1.0f),glm::vec3(0.0f,0.0f,-6.0f)); // Construye una matriz de translación con lo que yo le diga
	modelMat = glm::rotate(modelMat, angle, glm::vec3(1.0f, 0.0f, 0.0f)); // Yo altero la modelMatrix del cubo
	modelMat = glm::rotate(modelMat, angle, glm::vec3(0.0f, 1.0f, 0.0f)); // El orden de las matrices importan mucho en el movimiento!!!
	IGlib::setModelMat(objId, modelMat);*/
}

void keyboardFunc(unsigned char key, int x, int y)
{
	//Teclas del editor (1, +, -, X, Y, Z, R): si el editor usa la tecla, no se la pasamos a la cámara
	if (teclaEditor(key)) {
		return;
	}

	//MOVIMIENTO DE LA CÁMARA MEDIANTE TECLADO
	float cameraSpeed = 0.5f; //Distancia que sumamos o restamos para mover la cámara
	float rotationSpeed = 5.0f; //Ángulo que sumamos o restamos para girar la cámara

	//WASD para movimiento, Q y E para rotación
	if (key == 'W' || key == 'w') { //Movimiento hacia delante, sumamos a la posición frontal
		cameraPos += cameraSpeed * frontCam;
		targetPos += cameraSpeed * frontCam;
	}

	if (key == 'S' || key == 's') {	//Movimiento hacia atrás, restamos a la posición frontal
		cameraPos -= cameraSpeed * frontCam;
		targetPos -= cameraSpeed * frontCam;
	}

	if (key == 'D' || key == 'd') { //Movimiento hacia la derecha, sumamos a la derecha
		cameraPos += cameraSpeed * rightCam;
		targetPos += cameraSpeed * rightCam;
	}

	if (key == 'A' || key == 'a') { //Movimiento hacia la izquierda, restamos a la derecha
		cameraPos -= cameraSpeed * rightCam;
		targetPos -= cameraSpeed * rightCam;
	}

	if (key == 'Q' || key == 'q') { //Rotación hacia la izquierda, restamos al ángulo
		angHor -= rotationSpeed;
	}

	if (key == 'E' || key == 'e') { //Rotación hacia la derecha, sumamos al ángulo
		angHor += rotationSpeed;
	}

	//Movemos el targetPos para hacer que la cámara gire
	if (key == 'Q' || key == 'q' || key == 'E' || key == 'e') {
		glm::vec3 frontDirection;
		frontDirection.x = cos(glm::radians(angHor)); //El coseno calcula las coordenadas en el eje X
		frontDirection.y = 0.0f;
		frontDirection.z = sin(glm::radians(angHor)); //El seno calcula las coordenadas en el eje Z

		targetPos = cameraPos + glm::normalize(frontDirection);
	}

	// Actualizamos los vectores de la cámara
	frontCam = glm::normalize(targetPos - cameraPos);
	rightCam = glm::normalize(glm::cross(frontCam, upVector));
	upCam = glm::normalize(glm::cross(rightCam, frontCam));

	updateCameraVectors();
}

void mouseFunc(int button, int state, int x, int y)
{
	if (button == 2) { //Clic derecho: solo está pulsado entre el botón abajo (state 0) y arriba (state 1)
		rightClicPressed = (state == 0);
		firstMouse = true;
	}

	//Clic izquierdo (botón 0) al pulsar: seleccionamos el objeto que haya bajo el ratón
	if (button == 0 && state == 0) {
		seleccionarObjeto(x, y);
	}
}


void mouseMoveFunc(int x, int y)
{
	//MOVIMIENTO DE LA CÁMARA CON EL RATÓN
	if (!rightClicPressed) //Si el clic derecho no está pulsado, no se mueve la cámara
		return;

	if (firstMouse) { //Guarda las posiciones actuales del ratón al pulsar el clic derecho, para evitar saltos de posición
		lastX = x;
		lastY = y;
		firstMouse = false;
	}

	//Calculamos cuánto se ha movido el ratón (en píxeles)
	float xOffset = x - lastX;
	float yOffset = lastY - y;

	//Actualizamos las posiciones
	lastX = x;
	lastY = y;

	//Aplicamos sensibilidad
	float sensitivity = 0.1f;
	xOffset *= sensitivity; //Lo multiplicamos al offset para que el movimiento no sea muy brusco
	yOffset *= sensitivity;

	//Añadimos cuánto se ha movido la cámara a los ángulos horizontal y vertical
	angHor += xOffset;
	angVert += yOffset;

	//Limitamos los ángulos para no romperlo (Si el ángulo vertical llega a 90 grados, puede entrar en conflicto con el vector upVector y romper la matriz de vista)
	if (angVert > 89.0f)
		angVert = 89.0f;
	if (angVert < -89.9f)
		angVert = -89.0f;

	//Convertimos los ángulos a coordenadas para saber a dónde estamos mirando tras mover el ratón 
	glm::vec3 frontDirection;
	frontDirection.x = cos(glm::radians(angHor)) * cos(glm::radians(angVert)); //Coseno para calcular el eje x (deerecha-izquierda) y coseno para el eje y (la proyección del ángulo vertical en el eje horizontal)
	frontDirection.y = sin(glm::radians(angVert)); //La altura depende solo del ángulo vertical (con el seno)
	frontDirection.z = sin(glm::radians(angHor)) * cos(glm::radians(angVert)); //Seno para calcular el eje z (hacia delante- hacia atrás) y coseno para el eje y (arriba-abajo)

	//Al calcular dónde estamos mirando tras el movimiento, se lo sumamos a la posición actual para girar la cámara con el targetPos
	targetPos = cameraPos + glm::normalize(frontDirection);

	//Volvemos a calcular los vectores de la cámara
	frontCam = glm::normalize(targetPos - cameraPos);
	rightCam = glm::normalize(glm::cross(frontCam, upVector));
	upCam = glm::normalize(glm::cross(rightCam, frontCam));

	updateCameraVectors();
}

void mousePassMoveFunc(int x, int y)
{

}
void mouseWheelFunc(int wheel, int dir, int x, int y)
{

	//ZOOM CON LA RUEDA DEL RATÓN
	float zoomSpeed = 3.0f; //Ángulo que sumamos al hacer zoom

	if (dir > 0) {
		fov -= zoomSpeed; //Si vamos hacia arriba, el fov se disminuye (más cerca)
	} else {
		fov += zoomSpeed; //Si va hacia abajo, el fov aumenta (más lejos)
	}

	//Limitamos el fov para no romperlo :/
	if (fov < 1.0f)
		fov = 1.0f; // Zoom máximo
	if (fov > 120.0f)
		fov = 120.0f; //Zoom mínimo

	updateProjectionMatrix();
}

// ---------------------- Movimiento de cámara ---------------------- 
void updateCameraVectors() {
	glm::mat4 view = lookAtMatrix(cameraPos, targetPos, upVector);
	IGlib::setViewMat(view);
	viewMat = view; //Guardamos la vista para el picking
}

glm::mat4 lookAtMatrix(glm::vec3 pos, glm::vec3 target, glm::vec3 up) {
	glm::vec3 f = glm::normalize(target - pos);//front (eje Z)
	glm::vec3 r = glm::normalize(glm::cross(f, up)); //right (eje X)
	glm::vec3 u = glm::normalize(glm::cross(r, f));//up (eje Y)

	glm::mat4 view = glm::mat4(1.0f); //Inicializamos la matriz de vista

	//Rellenamos las primeras 3 columnas y filas (ROTACIÓN)
	//Eje X
	view[0][0] = r.x;
	view[1][0] = r.y;
	view[2][0] = r.z;

	//Eje Y
	view[0][1] = u.x;
	view[1][1] = u.y;
	view[2][1] = u.z;

	//Eje Z (en negativo porque la cámara debe mirar hacia la z negativa)
	view[0][2] = -f.x;
	view[1][2] = -f.y;
	view[2][2] = -f.z;

	//Rellenamos la última columna (TRASLACIÓN)
	view[3][0] = -glm::dot(r, pos);
	view[3][1] = -glm::dot(u, pos);
	view[3][2] = glm::dot(f, pos);

	return view;
}

void updateProjectionMatrix() {
	glm::mat4 proj = glm::mat4(1.0);

	float f = 1.0f / tan(glm::radians(fov) / 2.0); // Convertimos el fov a radianes y sacamos la tagente de la mitad
	float far = 20.0; 
	float near = 0.1; 

	proj[0].x = f/aspectRatio; 
	proj[1].y = f;
	proj[2].z = (far + near) / (near - far);
	proj[2].w = -1.0;
	proj[3].z = (2.0 * far * near) / (near - far);
	proj[3].w = 0.0;

	IGlib::setProjMat(proj);
	projMat = proj; //Guardamos la proyección para el picking
}


// ---------------------- Carga de objetos ---------------------- 

bool DoTheImportThing(const std::string& pFile, int &outObjId, float valorEscalado, glm::vec3 position) { 
	
	Assimp::Importer importer;
	sc = importer.ReadFile(pFile, 
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate | 
		aiProcess_JoinIdenticalVertices | 
		aiProcess_SortByPType);

		// Si falla import dime algo
	if (!sc) {
		std::cout << "Falló la carga" << std::endl;
		return false;
	}
	std::cout << "Se han cargado " << sc->mNumMeshes << " meshes" << std::endl;

	// Un modelo puede tener varias mallas (una por material): las agrupamos para escalarlas y seleccionarlas juntas
	Modelo modelo;
	modelo.boundsMin = glm::vec3(FLT_MAX);
	modelo.boundsMax = glm::vec3(-FLT_MAX);

	// bucle de carga 
	for (unsigned int n = 0; n < sc->mNumMeshes; ++n) {
		unsigned int* faceArray; 
		const aiMesh* mesh = sc->mMeshes[n];
		// Creo el array con un malloc (memory allocation)
		// Reserva memoria y cuando termina la reserva me devuelve un puntero. Fuerzo que ese puntero sea un unsigned int
		faceArray = (unsigned int*)malloc(sizeof(unsigned int) * mesh ->mNumFaces * 3); // *3 porque la cara es un triangulo
		unsigned int faceIndex = 0;

		float* vertexArray; 
		unsigned int vertexIndex = 0;

		float* normalArray;
		unsigned int normalIndex = 0;

		float* textCoordsArray;

		// Para rellenar los triángulos
		if (mesh->HasFaces()) { //por defecto siempre tiene, pero no está de más chequear
				for (unsigned int t = 0; t < mesh->mNumFaces; ++t) {
					const aiFace* face = &mesh->mFaces[t];
					memcpy(&faceArray[faceIndex], face->mIndices, 3 *
						sizeof(unsigned int)); // Copio los bytes que me hacen falta
					faceIndex += 3;
				}
		}
		
		if (mesh->HasPositions()) { //Idem: por defecto siempre tiene.
			vertexArray = (float*)malloc(sizeof(float) * mesh->mNumVertices * 3);

			for (unsigned int k = 0; k < mesh->mNumVertices; ++k) {
				const aiVector3D* vertex = &mesh->mVertices[k];
				memcpy(&vertexArray[vertexIndex], &float(vertex->x), sizeof(float));
				memcpy(&vertexArray[vertexIndex + 1], &float(vertex->y), sizeof(float));
				memcpy(&vertexArray[vertexIndex + 2], &float(vertex->z), sizeof(float));
				vertexIndex += 3; // Strife (avance de 3)
			}
		}

		if (mesh->HasNormals()) {
			normalArray = (float*)malloc(sizeof(float) * mesh->mNumVertices * 3);
			for (unsigned int k = 0; k < mesh->mNumVertices; ++k) {
				const aiVector3D* a_normal = &mesh->mNormals[k];
				memcpy(&normalArray[normalIndex], &float(a_normal->x), sizeof(float));
				memcpy(&normalArray[normalIndex + 1], &float(a_normal->y), sizeof(float));
				memcpy(&normalArray[normalIndex + 2], &float(a_normal->z), sizeof(float));
				normalIndex += 3;
			}
		}

		int id = IGlib::createObj(
			mesh->mNumFaces,
			mesh->mNumVertices,
			faceArray,
			vertexArray,
			0,
			normalArray
		);

		if (pFile.find("Firefly") != std::string::npos)
		{
			fireflyIds.push_back(id);
		}

		outObjId = id;
		modelo.ids.push_back(id); //Guardamos el ID de cada malla

		//Ampliamos la caja del modelo con los vértices de esta malla
		for (unsigned int k = 0; k < mesh->mNumVertices; ++k) {
			glm::vec3 v(vertexArray[3 * k], vertexArray[3 * k + 1], vertexArray[3 * k + 2]);
			modelo.boundsMin = glm::min(modelo.boundsMin, v);
			modelo.boundsMax = glm::max(modelo.boundsMax, v);
		}
	}

	//Para que los objetos queden donde estaban, multiplicamos la posición por la escala y usamos traslación * escalado 
	modelo.posicion = position * valorEscalado;
	modelo.posicionInicial = modelo.posicion;
	modelo.escala = glm::vec3(valorEscalado, valorEscalado, valorEscalado);
	modelo.rotacion = glm::mat4(1.0f); //Sin rotación al cargar
	modelo.escalaInicial = valorEscalado;
	if (pFile.find("Firefly") != std::string::npos)
		indiceLuciernaga = (int)modelos.size(); //Guardamos cuál es la luciérnaga (su posición la mueve idleFunc)
	modelos.push_back(modelo);
	actualizarModelo(modelo); //dibuja el objeto en la posicion y escalado que se le ha dado antes 

	return true;
}


// ---------------------- Selección y escalado de objetos ---------------------- 

glm::mat4 matrizModelo(const Modelo& m) {
	glm::mat4 mat = glm::translate(glm::mat4(1.0f), m.posicion); // Primero la traslación
	mat = mat * m.rotacion; // luego la rotación
	mat = glm::scale(mat, m.escala); // y el escalado se aplica sobre los ejes del propio objeto
	return mat;
}

void actualizarModelo(const Modelo& m) {
	glm::mat4 mat = matrizModelo(m);
	for (size_t i = 0; i < m.ids.size(); ++i) //Todas las mallas del modelo comparten la misma matriz
		IGlib::setModelMat(m.ids[i], mat);
}

void seleccionarObjeto(int mx, int my) {
	//Convertimos el pixel del clic en un rayo: píxeles -> coordenadas de -1 a 1 -> mundo (deshaciendo proyección y vista)
	float nx = 2.0f * mx / anchoVentana - 1.0f;
	float ny = 1.0f - 2.0f * my / altoVentana; //En la ventana Y crece hacia abajo, en OpenGL hacia arriba
	glm::mat4 inversa = glm::inverse(projMat * viewMat);
	glm::vec4 cercano = inversa * glm::vec4(nx, ny, -1.0f, 1.0f); //Punto del rayo en el plano near
	glm::vec4 lejano = inversa * glm::vec4(nx, ny, 1.0f, 1.0f); //Punto del rayo en el plano far
	glm::vec3 origen = glm::vec3(cercano) / cercano.w;
	glm::vec3 dir = glm::vec3(lejano) / lejano.w - origen; //Sin normalizar: t = 0 en near y t = 1 en far

	int elegido = -1;
	float mejorT = 2.0f;
	for (size_t i = 0; i < modelos.size(); ++i) {
		//Pasamos el rayo al espacio del propio modelo: así la caja siempre está alineada con los ejes, aunque el modelo esté escalado
		glm::mat4 inv = glm::inverse(matrizModelo(modelos[i]));
		glm::vec3 oLocal = glm::vec3(inv * glm::vec4(origen, 1.0f));
		glm::vec3 dLocal = glm::vec3(inv * glm::vec4(dir, 0.0f));
		float t;
		if (rayoCaja(oLocal, dLocal, modelos[i].boundsMin, modelos[i].boundsMax, t) && t < mejorT) {
			mejorT = t; //Nos quedamos con el modelo más cercano a la cámara
			elegido = (int)i;
		}
	}

	seleccionado = elegido;
	ejeEscalado = 0;
	if (seleccionado < 0) {
		modoEscalado = false;
		modoRotacion = false;
		modoTraslacion = false;
		std::cout << "Nada seleccionado" << std::endl;
	}
	else {
		std::cout << "Seleccionado el modelo " << seleccionado << " (pulsa 1 para escalar, 2 para rotar o 3 para mover)" << std::endl;
	}
}

bool teclaEditor(unsigned char key) {
	if (key == '1') { //Activa el modo escalado
		if (seleccionado < 0) { std::cout << "Selecciona primero un objeto con el clic izquierdo" << std::endl; return true; }
		modoEscalado = true;
		modoRotacion = false;
		modoTraslacion = false;
		ejeEscalado = 0; //Al escalar, por defecto se escalan los tres ejes
		std::cout << "Modo escalado: + y - para cambiar el tamaño, X/Y/Z para elegir eje, R para resetear" << std::endl;
		return true;
	}

	if (key == '2') { //Activa el modo rotación
		if (seleccionado < 0) { std::cout << "Selecciona primero un objeto con el clic izquierdo" << std::endl; return true; }
		modoRotacion = true;
		modoEscalado = false;
		modoTraslacion = false;
		if (ejeEscalado == 0) ejeEscalado = 2; //Al rotar, por defecto se gira sobre el eje Y
		std::cout << "Modo rotacion: + y - para girar, X/Y/Z para elegir eje, R para resetear" << std::endl;
		return true;
	}

	if (key == '3') { //Activa el modo traslación
		if (seleccionado < 0) { std::cout << "Selecciona primero un objeto con el clic izquierdo" << std::endl; return true; }
		modoTraslacion = true;
		modoEscalado = false;
		modoRotacion = false;
		if (ejeEscalado == 0) ejeEscalado = 1; //Al mover, por defecto se mueve sobre el eje X
		std::cout << "Modo traslacion: + y - para mover, X/Y/Z para elegir eje, R para resetear" << std::endl;
		return true;
	}

	if (key == 'x' || key == 'X' || key == 'y' || key == 'Y' || key == 'z' || key == 'Z') { //Elegir el eje
		if (seleccionado < 0) return true;
		int eje = (key == 'x' || key == 'X') ? 1 : ((key == 'y' || key == 'Y') ? 2 : 3);
		if (modoRotacion || modoTraslacion)
			ejeEscalado = eje; //Al rotar siempre hace falta un eje concreto
		else
			ejeEscalado = (ejeEscalado == eje) ? 0 : eje; //Al escalar, pulsar el mismo eje otra vez vuelve a los tres
		const char* nombres[] = { "XYZ", "X", "Y", "Z" };
		std::cout << "Eje: " << nombres[ejeEscalado] << std::endl;
		return true;
	}

	if (key == '+' || key == '=') { //Agrandar o girar en un sentido (el '=' es la misma tecla que '+' sin pulsar mayúsculas)
		if (seleccionado >= 0 && modoEscalado) escalarSeleccionado(1.1f);
		if (seleccionado >= 0 && modoRotacion) rotarSeleccionado(5.0f);
		if (seleccionado >= 0 && modoTraslacion) trasladarSeleccionado(0.25f);
		return true;
	}

	if (key == '-' || key == '_') { //Reducir o girar en sentido contrario
		if (seleccionado >= 0 && modoEscalado) escalarSeleccionado(1.0f / 1.1f);
		if (seleccionado >= 0 && modoRotacion) rotarSeleccionado(-5.0f);
		if (seleccionado >= 0 && modoTraslacion) trasladarSeleccionado(-0.25f);
		return true;
	}

	if (key == 'r' || key == 'R') { //Volver al valor original de lo que se está editando
		if (seleccionado < 0) return true;
		if (modoRotacion) {
			modelos[seleccionado].rotacion = glm::mat4(1.0f); //Sin rotación
		}
		else if (modoTraslacion) {
			modelos[seleccionado].posicion = modelos[seleccionado].posicionInicial; //Posición original
		}
		else {
			float original = modelos[seleccionado].escalaInicial; //Escala con la que se cargó el modelo
			modelos[seleccionado].escala = glm::vec3(original);
		}
		actualizarModelo(modelos[seleccionado]);
		return true;
	}

	return false; //No es una tecla del editor: la usa la cámara
}

void escalarSeleccionado(float factor) {
	float minimo = modelos[seleccionado].escalaInicial / 20.0f; //Límites relativos a la escala de carga: entre 1/20 y 20 veces,
	float maximo = modelos[seleccionado].escalaInicial * 20.0f; //para que el objeto no desaparezca ni sea gigante
	glm::vec3& e = modelos[seleccionado].escala;
	if (ejeEscalado == 0 || ejeEscalado == 1) e.x = glm::clamp(e.x * factor, minimo, maximo);
	if (ejeEscalado == 0 || ejeEscalado == 2) e.y = glm::clamp(e.y * factor, minimo, maximo);
	if (ejeEscalado == 0 || ejeEscalado == 3) e.z = glm::clamp(e.z * factor, minimo, maximo);
	actualizarModelo(modelos[seleccionado]);
	std::cout << "Escala: (" << e.x << ", " << e.y << ", " << e.z << ")" << std::endl;
}

void rotarSeleccionado(float grados) {
	glm::vec3 eje(0.0f, 1.0f, 0.0f); //Por defecto, eje Y
	if (ejeEscalado == 1) eje = glm::vec3(1.0f, 0.0f, 0.0f);
	if (ejeEscalado == 3) eje = glm::vec3(0.0f, 0.0f, 1.0f);

	//Acumulamos el giro sobre los ejes del propio objeto (multiplicando por la derecha), igual que el escalado
	glm::mat4 giro = glm::rotate(glm::mat4(1.0f), glm::radians(grados), eje);
	modelos[seleccionado].rotacion = modelos[seleccionado].rotacion * giro;
	actualizarModelo(modelos[seleccionado]);
	std::cout << "Rotado " << grados << " grados" << std::endl;
}

void trasladarSeleccionado(float distancia) {
	if (seleccionado == indiceLuciernaga) { //idleFunc recoloca la luciérnaga en cada frame, así que no se puede mover a mano
		std::cout << "La luciernaga se mueve sola alrededor de la moto, no se puede trasladar" << std::endl;
		return;
	}

	glm::vec3 eje(1.0f, 0.0f, 0.0f); //Por defecto, eje X
	if (ejeEscalado == 2) eje = glm::vec3(0.0f, 1.0f, 0.0f);
	if (ejeEscalado == 3) eje = glm::vec3(0.0f, 0.0f, 1.0f);

	//Se mueve sobre los ejes del mundo (no los del objeto, que puede estar rotado)
	glm::vec3& p = modelos[seleccionado].posicion;
	p += eje * distancia;
	actualizarModelo(modelos[seleccionado]);
	std::cout << "Posicion: (" << p.x << ", " << p.y << ", " << p.z << ")" << std::endl;
}

//Intersección entre el rayo (o + t*d, con t entre 0 y 1) y una caja alineada con los ejes
bool rayoCaja(glm::vec3 o, glm::vec3 d, glm::vec3 bmin, glm::vec3 bmax, float& t) {
	float tmin = 0.0f, tmax = 1.0f;
	for (int i = 0; i < 3; ++i) { //Un par de planos por cada eje
		if (fabs(d[i]) < 1e-8f) {
			if (o[i] < bmin[i] || o[i] > bmax[i]) return false; //Rayo paralelo a los planos y fuera de ellos
		}
		else {
			float t1 = (bmin[i] - o[i]) / d[i];
			float t2 = (bmax[i] - o[i]) / d[i];
			if (t1 > t2) { float aux = t1; t1 = t2; t2 = aux; }
			if (t1 > tmin) tmin = t1;
			if (t2 < tmax) tmax = t2;
			if (tmin > tmax) return false; //Los intervalos no se solapan: el rayo no toca la caja
		}
	}
	t = tmin;
	return true;
}

