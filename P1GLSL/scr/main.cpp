#include "BOX.h"
#include <IGL/IGlib.h>
#include <vector>
#include <cfloat>     // FLT_MAX: límites de las cajas envolventes para selección
#include <cmath>      // Funciones trigonométricas usadas en las animaciones y la cámara

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#define M_PI 3.14159265

// Assimp library
#include <assimp/Importer.hpp>  // Para la importación de ficheros
#include <assimp/scene.h> // Estructura de la escena
#include <assimp/postprocess.h> // Flags de procesado


// ---------------------- Variables ----------------------
//Idenficadores de los objetos de la escena
int objId = -1;

// Variable timer para animación en shaders
static unsigned int my_timer;

//Variables para movimiento de objetos
float fireflyAngle = 0.0f;
float fireflyRadius = 0.5f;
glm::vec3 centre = glm::vec3(-0.4f, 0.0f, -2.0f);

// Variables para movimiento de la cámara
glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 2.0f); // Posición de la cámara en el espacio
glm::vec3 targetPos = glm::vec3(0.0f, 1.0f, -1.0f); // Hacia donde mira la cámara
glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f); //Vector hacia arriba (eje Y positivo)

float angHor = -90.0f; //Ángulo inicial de la cámara (horizontal). Comenzamos en -90 para que la cámara comience mirando hacia delante
float angVert = 0.0f; //Ángulo inicial de la cámara (vertical)

glm::vec3 frontCam; //Vector que apunta delante de la cámara
glm::vec3 rightCam; //Vector que apunta a la derecha de la cámara
glm::vec3 upCam; //Vector que apunta arriba de la cámara

// Variables para controlar el movimiento del ratón
float lastX = 400.0f;
float lastY = 300.0f;
bool firstMouse = true; // Nos ayuda a reducir saltos incómodos al mover la cámara
bool rightClicPressed; // Detecta si se está pulsando el clic derecho para el movimiento de cámara

// Variables para zoom
float fov = 60.0f; //Ángulo inicial de visión
float aspectRatio = 4.0f / 3.0f; //Valor inicial; se actualiza al redimensionar la ventana

// Copias de las matrices y dimensiones de ventana necesarias para seleccionar objetos
glm::mat4 viewMat = glm::mat4(1.0f);
glm::mat4 projMat = glm::mat4(1.0f);
int anchoVentana = 800;
int altoVentana = 600;

// Modelos cargados
// Un modelo cargado puede tener varias mallas (una por material), así que se guardan todas juntas
struct Model {
	std::vector<int> ids; // Todas las mallas de un mismo modelo
	glm::vec3 position;   // Posición actual
	glm::vec3 scale;      // Escala actual
	glm::vec3 rotation;   // Rotación base de la práctica 2 (ángulos de Euler en grados)

	glm::mat4 editRotation = glm::mat4(1.0f); // Giros del editor sobre los ejes locales
	glm::vec3 originalPosition = glm::vec3(0.0f);
	float originalScale = 1.0f;
	glm::vec3 boundsMin = glm::vec3(FLT_MAX);  // Caja en espacio local
	glm::vec3 boundsMax = glm::vec3(-FLT_MAX);
	bool selectable = true; // La skybox no se debe poder seleccionar
};

std::vector<Model> models; // Todos los modelos de la escena

// Índice del modelo animado
int fireflyIndex = -1;

int seleccionado = -1; // -1 indica que no hay ningún objeto seleccionado
bool modoEscalado = false;
bool modoRotacion = false;
bool modoTraslacion = false;
int ejeEscalado = 0; // 0 = XYZ para escalado; 1 = X, 2 = Y, 3 = Z

// Texturas por defecto, por si el objeto cargado no tiene
const std::string texBlackDefault = "../3D_assets/blackDefault.png";
const std::string texNormalDefault = "../3D_assets/normalDefault.png";

// ---------------------- Declaración de métodos ---------------------- 
//Declaración de CB
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

//Matriz de modelo
glm::mat4 modelMatrix(const Model& m); //Matriz de modelo
void updateModel(const Model& m); //Envía la matriz de modelo a todas las mallas del modelo

void seleccionarObjeto(int mx, int my);
bool teclaEditor(unsigned char key);
void escalarSeleccionado(float factor);
void rotarSeleccionado(float grados);
void trasladarSeleccionado(float distancia);
bool rayoCaja(glm::vec3 o, glm::vec3 d, glm::vec3 bmin, glm::vec3 bmax, float& t);

// Carga de objetos
bool DoTheImportThing(const std::string& pFile, float scaleValue, glm::vec3 position, glm::vec3 rotation, const std::string& colorTex, const std::string& emiTex = "",
	const std::string& normalTex = "", const std::string& specularTex = "", const std::string& auxiliarTex = "");

// Creación de una skybox de cielo nocturno
void createSkybox();

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

	my_timer = 0; //Inicialización del timer
	IGlib::setTimer(my_timer);

	// Carga de objetos
		// Moto
	if (!DoTheImportThing("../3D_assets/Bike/Bike.obj", 0.7f, glm::vec3(0.3f, 0.0f, -1.0f), glm::vec3(0.0, -30.0, 0.0f), "../3D_assets/Bike/Bike_Textures/Bike_Atlas_BaseColor.png",
		"../3D_assets/Bike/Bike_Textures/Bike_Atlas_Emissive.png", "../3D_assets/Bike/Bike_Textures/Bike_Atlas_Normal.png",
		"../3D_assets/Bike/Bike_Textures/Bike_Atlas_Specular.png", "../3D_assets/Bike/Bike_Textures/Bike_Atlas_Roughness.png")) return -1;
	// Luciernaga
	if (!DoTheImportThing("../3D_assets/Firefly/Firefly.obj", 0.02f, glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f), "../3D_assets/Firefly/Firefly_Textures/firefly_albedo.png",
		"../3D_assets/Firefly/Firefly_Textures/firefly_emissive.png", "../3D_assets/Firefly/Firefly_Textures/firefly_normal.png",
		"../3D_assets/Firefly/Firefly_Textures/firefly_bump.png", "../3D_assets/Firefly/Firefly_Textures/firefly_opacity.png")) return -1;
	fireflyIndex = models.size() - 1;
	// Espada
	if (!DoTheImportThing("../3D_assets/Sword/Sword.obj", 1.0f, glm::vec3(0.49f, 0.0f, -0.25f), glm::vec3(-10.0f, 0.0f, 15.0f), "../3D_assets/Sword/Sword_Textures/Espada_Diffuse.jpg",
		"../3D_assets/Sword/Sword_Textures/Espada_Emissive.jpeg", "../3D_assets/Sword/Sword_Textures/Espada_Normal.jpg", "../3D_assets/Sword/Sword_Textures/Espada_Specular.jpg", "../3D_assets/Sword/Sword_Textures/Espada_Roughness.jpg")) return -1;
	// Farola
	if (!DoTheImportThing("../3D_assets/Lamp/lamp.obj", 0.37f, glm::vec3(-1.6f, 0.0f, 0.0f), glm::vec3(0.0f, 180.0f, 0.0f), "../3D_assets/Lamp/Lamp_Textures/lamp_diffuse.png",
		"../3D_assets/Lamp/Lamp_Textures/lamp_emissive.png", "../3D_assets/Lamp/Lamp_Textures/lamp_normal.png", "../3D_assets/Lamp/Lamp_Textures/lamp_specular.png", "")) return -1;
	// Arbusto
	if (!DoTheImportThing("../3D_assets/Bush/bush.obj", 0.8f, glm::vec3(-0.4f, 0.0f, -2.5f), glm::vec3(0.0f, 0.0f, 0.0f), "../3D_assets/Bush/Bush_Textures/bush_diffuse.png",
		"", "../3D_assets/Bush/Bush_Textures/bush_normal.png", "../3D_assets/Bush/Bush_Textures/bush_specular.png", "")) return -1;

	createSkybox();

	// CBs (Callbacks)
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
	anchoVentana = width;
	altoVentana = height;
	updateProjectionMatrix();
}

void idleFunc()
{
	//Timer (animación en shader)
	my_timer++;
	IGlib::setTimer(my_timer);

	//Movimiento de la luciérnaga (que depende del tiempo)
	fireflyAngle = fmod(my_timer * 0.001f, 2.0f * M_PI);

	glm::vec3 fireflyPos;

	fireflyPos.x = centre.x + cos(fireflyAngle) * fireflyRadius;
	fireflyPos.z = centre.z + sin(fireflyAngle) * fireflyRadius;

	// Pequeño movimiento vertical para simular vuelo
	fireflyPos.y = centre.y + 1.0f + sin(fireflyAngle * 3.0f) * 0.3f;

	//Actualizamos la posición de la luciérnaga manteniendo su escala
	if (fireflyIndex >= 0) {
		models[fireflyIndex].position = fireflyPos;
		models[fireflyIndex].rotation.y = -glm::degrees(fireflyAngle) - 90.0f; //Hacemos que la luciernaga mire hacia donde vuela (+90 para incluir un desfase; sino, vuela de lado)
		updateModel(models[fireflyIndex]);
	}
}

void keyboardFunc(unsigned char key, int x, int y)
{
	// El editor procesa primero sus teclas para evitar conflictos con la cámara
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

	// Movemos el targetPos para hacer que la cámara gire
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
	if (button == 0 && state == 0) { // Clic izquierdo: selección del objeto
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
	if (angVert < -89.0f)
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

void mousePassMoveFunc(int x, int y){}

void mouseWheelFunc(int wheel, int dir, int x, int y)
{
	//ZOOM CON LA RUEDA DEL RATÓN
	float zoomSpeed = 3.0f; //Ángulo que sumamos al hacer zoom

	if (dir > 0) {
		fov -= zoomSpeed; //Si vamos hacia arriba, el fov se disminuye (más cerca)
	}
	else {
		fov += zoomSpeed; //Si va hacia abajo, el fov aumenta (más lejos)
	}

	//Limitamos el fov
	if (fov < 1.0f)
		fov = 1.0f; // Zoom máximo
	if (fov > 120.0f)
		fov = 120.0f; //Zoom mínimo

	updateProjectionMatrix();
}

// ---------------------- MOVIMIENTO DE CÁMARA ----------------------
void updateCameraVectors() {
	glm::mat4 view = lookAtMatrix(cameraPos, targetPos, upVector);
	IGlib::setViewMat(view);
	viewMat = view; // Matriz usada por el picking
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
	float far = 50.0;
	float near = 0.1;

	proj[0].x = f / aspectRatio;
	proj[1].y = f;
	proj[2].z = (far + near) / (near - far);
	proj[2].w = -1.0;
	proj[3].z = (2.0 * far * near) / (near - far);
	proj[3].w = 0.0;

	IGlib::setProjMat(proj);
	projMat = proj; // Matriz usada por el picking
}

// ---------------------- CARGA DE OBJETOS ----------------------
bool DoTheImportThing(const std::string& pFile, float valorEscalado, glm::vec3 position, glm::vec3 rotacion, const std::string& colorTex, const std::string& emiTex
	, const std::string& normalTex, const std::string& specularTex, const std::string& auxiliarTex) {

	// Declaramos sc como variable local
	const aiScene* sc;

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

	std::cout << "Se han cargado " << sc->mNumMeshes << " mesh(es)" << std::endl;

	// Un modelo puede tener varias mallas (una por material): las agrupamos 
	Model modelo;
	modelo.boundsMin = glm::vec3(FLT_MAX);
	modelo.boundsMax = glm::vec3(-FLT_MAX);

	// Bucle de carga
	for (unsigned int n = 0; n < sc->mNumMeshes; ++n) {
		unsigned int* faceArray;
		const aiMesh* mesh = sc->mMeshes[n];

		if (mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE) continue; //Ignora si hay alguna primitiva que no sea un triángulo

		// Reserva memoria y cuando termina la reserva me devuelve un puntero. Fuerzo que ese puntero sea un unsigned int
		faceArray = (unsigned int*)malloc(sizeof(unsigned int) * mesh->mNumFaces * 3);
		unsigned int faceIndex = 0;

		float* vertexArray;
		unsigned int vertexIndex = 0;
		unsigned int normalIndex = 0;

		// Para rellenar los triángulos
		if (mesh->HasFaces()) {
			for (unsigned int t = 0; t < mesh->mNumFaces; ++t) {
				const aiFace* face = &mesh->mFaces[t];
				memcpy(&faceArray[faceIndex], face->mIndices, 3 * sizeof(unsigned int));
				faceIndex += 3;
			}
		}

		if (mesh->HasPositions()) {
			vertexArray = (float*)malloc(sizeof(float) * mesh->mNumVertices * 3);

			for (unsigned int k = 0; k < mesh->mNumVertices; ++k) {
				const aiVector3D* vertex = &mesh->mVertices[k];
				memcpy(&vertexArray[vertexIndex], &float(vertex->x), sizeof(float));
				memcpy(&vertexArray[vertexIndex + 1], &float(vertex->y), sizeof(float));
				memcpy(&vertexArray[vertexIndex + 2], &float(vertex->z), sizeof(float));
				vertexIndex += 3; // Strife (avance de 3)
			}
		}

		// Array de normales
		float* normalArray = NULL;
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

		// Array de oordenadas de textura: 2 valores (u, v) por vértice. Solo usamos el primer juego (el 0)
		float* texCoordArray = NULL;
		if (mesh->HasTextureCoords(0)) {
			texCoordArray = (float*)malloc(sizeof(float) * mesh->mNumVertices * 2);
			for (unsigned int k = 0; k < mesh->mNumVertices; ++k) {
				texCoordArray[2 * k] = mesh->mTextureCoords[0][k].x;
				texCoordArray[2 * k + 1] = mesh->mTextureCoords[0][k].y;
			}
		}

		// Array de tangentes 
		float* tangentArray = NULL;
		if (mesh->HasTangentsAndBitangents()) {
			tangentArray = (float*)malloc(sizeof(float) * mesh->mNumVertices * 3);
			for (unsigned int k = 0; k < mesh->mNumVertices; ++k) {
				tangentArray[3 * k] = mesh->mTangents[k].x;
				tangentArray[3 * k + 1] = mesh->mTangents[k].y;
				tangentArray[3 * k + 2] = mesh->mTangents[k].z;
			}
		}

		int id = IGlib::createObj(
			mesh->mNumFaces,
			mesh->mNumVertices,
			faceArray,
			vertexArray,
			0, // color
			normalArray,
			texCoordArray,
			tangentArray
		);

		IGlib::addColorTex(id, colorTex.c_str()); //Textura de color (difusa) de todas las mallas del modelo
		if (!emiTex.empty())
			IGlib::addEmissiveTex(id, emiTex.c_str());
		else
			IGlib::addEmissiveTex(id, texBlackDefault.c_str());
		if (!normalTex.empty())
			IGlib::addNormalTex(id, normalTex.c_str());
		else
			IGlib::addNormalTex(id, texNormalDefault.c_str());
		if (!specularTex.empty())
			IGlib::addSpecularTex(id, specularTex.c_str());
		if (!auxiliarTex.empty())
			IGlib::addAuxiliarTex(id, auxiliarTex.c_str());
		else
			IGlib::addAuxiliarTex(id, colorTex.c_str()); // Si no hay textura auxiliar, usamos el color (para la iluminación en el shader)

		modelo.ids.push_back(id); //Guardamos el ID de cada malla

		// Ampliamos la AABB local con los vértices de todas las mallas del modelo
		if (mesh->HasPositions()) {
			for (unsigned int k = 0; k < mesh->mNumVertices; ++k) {
				glm::vec3 v(mesh->mVertices[k].x, mesh->mVertices[k].y, mesh->mVertices[k].z);
				modelo.boundsMin = glm::min(modelo.boundsMin, v);
				modelo.boundsMax = glm::max(modelo.boundsMax, v);
			}
		}
	}

	//Para que los objetos queden donde estaban, multiplicamos la posición por la escala y usamos traslación * rotación * escalado
	modelo.position = position * valorEscalado;
	modelo.rotation = rotacion;
	modelo.scale = glm::vec3(valorEscalado, valorEscalado, valorEscalado);
	modelo.originalPosition = modelo.position;
	modelo.originalScale = valorEscalado;
	modelo.editRotation = glm::mat4(1.0f);

	models.push_back(modelo);
	updateModel(modelo); //Dibuja el objeto en la posicion y escalado que se le ha dado antes

	return true;
}

// ---------------------- MATRIZ DE MODELO ----------------------
glm::mat4 modelMatrix(const Model& m) {
	glm::mat4 mat = glm::translate(glm::mat4(1.0f), m.position); // Traslación
	mat = glm::rotate(mat, glm::radians(m.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotación (X)
	mat = glm::rotate(mat, glm::radians(m.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotación (Y)
	mat = glm::rotate(mat, glm::radians(m.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotación (Z)
	mat = mat * m.editRotation; // Rotación interactiva local sin perder la rotación inicial o animada
	mat = glm::scale(mat, m.scale); // Escalado
	return mat;
}

void updateModel(const Model& m) {
	glm::mat4 mat = modelMatrix(m);
	for (size_t i = 0; i < m.ids.size(); ++i) //Todas las mallas del modelo comparten la misma matriz
		IGlib::setModelMat(m.ids[i], mat);
}

void createSkybox() {
	// Copiamos los indices y los giramos (0,1,2 -> 0,2,1) para que las caras miren hacia dentro
	std::vector<unsigned int> indices(cubeTriangleIndex, cubeTriangleIndex + cubeNTriangleIndex * 3);
	for (size_t i = 0; i < indices.size(); i += 3)
		std::swap(indices[i + 1], indices[i + 2]);

	// Normales hacia dentro
	std::vector<float> normales(cubeVertexNormal, cubeVertexNormal + cubeNVertex * 3);
	for (float& n : normales)
		n = -n;

	int id = IGlib::createObj(cubeNTriangleIndex, cubeNVertex, indices.data(),
		cubeVertexPos, cubeVertexColor, normales.data(), cubeVertexTexCoord, cubeVertexTangent);

	// Texturas 
	IGlib::addColorTex(id, texBlackDefault.c_str());
	IGlib::addAuxiliarTex(id, texBlackDefault.c_str());
	IGlib::addEmissiveTex(id, "../3D_assets/nightSky.png");
	IGlib::addSpecularTex(id, texBlackDefault.c_str());
	IGlib::addNormalTex(id, texNormalDefault.c_str());

	Model caja;
	caja.ids.push_back(id);
	caja.position = glm::vec3(0.0f);
	caja.rotation = glm::vec3(0.0f);
	caja.scale = glm::vec3(15.0f);
	caja.selectable = false; // No permitir que la skybox tape la selección de objetos
	models.push_back(caja);
	updateModel(caja);
}

// ---------------------- Editor de objetos ----------------------
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
	for (size_t i = 0; i < models.size(); ++i) {
		if (!models[i].selectable || models[i].ids.empty()) continue; // Ignorar la skybox y modelos vacíos
		//Pasamos el rayo al espacio del propio modelo: así la caja siempre está alineada con los ejes, aunque el modelo esté escalado
		glm::mat4 inv = glm::inverse(modelMatrix(models[i]));
		glm::vec3 oLocal = glm::vec3(inv * glm::vec4(origen, 1.0f));
		glm::vec3 dLocal = glm::vec3(inv * glm::vec4(dir, 0.0f));
		float t;
		if (rayoCaja(oLocal, dLocal, models[i].boundsMin, models[i].boundsMax, t) && t < mejorT) {
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
			models[seleccionado].editRotation = glm::mat4(1.0f); //Sin rotación
		}
		else if (modoTraslacion) {
			models[seleccionado].position = models[seleccionado].originalPosition; //Posición original
		}
		else {
			float original = models[seleccionado].originalScale; //Escala con la que se cargó el modelo
			models[seleccionado].scale = glm::vec3(original);
		}
		updateModel(models[seleccionado]);
		return true;
	}

	return false; //No es una tecla del editor: la usa la cámara
}

void escalarSeleccionado(float factor) {
	float minimo = models[seleccionado].originalScale / 20.0f; //Límites relativos a la escala de carga: entre 1/20 y 20 veces,
	float maximo = models[seleccionado].originalScale * 20.0f; //para que el objeto no desaparezca ni sea gigante
	glm::vec3& e = models[seleccionado].scale;
	if (ejeEscalado == 0 || ejeEscalado == 1) e.x = glm::clamp(e.x * factor, minimo, maximo);
	if (ejeEscalado == 0 || ejeEscalado == 2) e.y = glm::clamp(e.y * factor, minimo, maximo);
	if (ejeEscalado == 0 || ejeEscalado == 3) e.z = glm::clamp(e.z * factor, minimo, maximo);
	updateModel(models[seleccionado]);
	std::cout << "Escala: (" << e.x << ", " << e.y << ", " << e.z << ")" << std::endl;
}

void rotarSeleccionado(float grados) {
	glm::vec3 eje(0.0f, 1.0f, 0.0f); //Por defecto, eje Y
	if (ejeEscalado == 1) eje = glm::vec3(1.0f, 0.0f, 0.0f);
	if (ejeEscalado == 3) eje = glm::vec3(0.0f, 0.0f, 1.0f);

	//Acumulamos el giro sobre los ejes del propio objeto (multiplicando por la derecha), igual que el escalado
	glm::mat4 giro = glm::rotate(glm::mat4(1.0f), glm::radians(grados), eje);
	models[seleccionado].editRotation = models[seleccionado].editRotation * giro;
	updateModel(models[seleccionado]);
	std::cout << "Rotado " << grados << " grados" << std::endl;
}

void trasladarSeleccionado(float distancia) {
	if (seleccionado == fireflyIndex) { //idleFunc recoloca la luciérnaga en cada frame, así que no se puede mover a mano
		std::cout << "La luciernaga se mueve sola alrededor de la moto, no se puede trasladar" << std::endl;
		return;
	}

	glm::vec3 eje(1.0f, 0.0f, 0.0f); //Por defecto, eje X
	if (ejeEscalado == 2) eje = glm::vec3(0.0f, 1.0f, 0.0f);
	if (ejeEscalado == 3) eje = glm::vec3(0.0f, 0.0f, 1.0f);

	//Se mueve sobre los ejes del mundo (no los del objeto, que puede estar rotado)
	glm::vec3& p = models[seleccionado].position;
	p += eje * distancia;
	updateModel(models[seleccionado]);
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