#include "BOX.h"
#include <IGL/IGlib.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
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

// Variables assimp
const aiScene* sc; 

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

	if (!DoTheImportThing("../3D_assets/Firefly/Firefly.obj", objeto2Id, 0.15f, glm::vec3(2.5f, -6.8f, -2.0f))) {
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
	updateProjectionMatrix();
}

void idleFunc()
{
	/*static float angle = 0.0f;

	angle = (angle < M_PI * 2.0) ? angle + 0.001 : 0.0f; // Si el ángulo es más de 60 grados, lo amplío, y sino reseteo 0.0. Hará que esté continuamente en rotación

	glm::mat4 modelMat = glm::translate(glm::mat4(1.0f),glm::vec3(0.0f,0.0f,-6.0f)); // Construye una matriz de translación con lo que yo le diga
	modelMat = glm::rotate(modelMat, angle, glm::vec3(1.0f, 0.0f, 0.0f)); // Yo altero la modelMatrix del cubo
	modelMat = glm::rotate(modelMat, angle, glm::vec3(0.0f, 1.0f, 0.0f)); // El orden de las matrices importan mucho en el movimiento!!!
	IGlib::setModelMat(objId, modelMat);*/
}

void keyboardFunc(unsigned char key, int x, int y)
{
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
	if (button == 2) { //Clic derecho
		rightClicPressed = true;
		firstMouse = true;
	}
	else {
		rightClicPressed = false;
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

		outObjId = IGlib::createObj(mesh->mNumFaces, mesh->mNumVertices, faceArray, vertexArray, 0, normalArray);
		glm::mat4 modelMatObjeto = glm::mat4(1.0f); //se crea una matriz de transformacion 4x4
		modelMatObjeto = glm::scale(modelMatObjeto, glm::vec3(valorEscalado, valorEscalado, valorEscalado));   // Escalado del objeto
		modelMatObjeto = glm::translate(modelMatObjeto, position); // Posición 
		IGlib::setModelMat(outObjId, modelMatObjeto); //dibuja el objeto en la posicion y escalado que se le ha dado antes 

	}

	return true;
}

