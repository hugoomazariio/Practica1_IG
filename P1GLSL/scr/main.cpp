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



//Idenficadores de los objetos de la escena
int objId = -1;

// Variables assimp
const aiScene* sc; 


//Declaraci�n de CB
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);
void mouseMoveFunc(int x, int y);
void mousePassMoveFunc(int x, int y);
void mouseWheelFunc(int wheel, int dir, int x, int y);

// Carga de objetos
bool DoTheImportThing(const std::string& pFile);


int main(int argc, char** argv)
{
	std::locale::global(std::locale("spanish"));// acentos ;)
	if (!IGlib::init("../shaders_P1/shader.v7.vert", "../shaders_P1/shader.v7.frag")) 
		return -1;
	
	
	
	//Se ajusta la c�mara
	//Si no se da valor se cojen valores por defecto
	
	//model_view matrix
	glm::mat4 view = glm::mat4(1.0); // Inicializo la matriz como matriz identidad
										  // Al pasarlo a c++ mat4 no está incluido, tengo que poner glm::mat4 defino la matriz 4x4

	// Ahora posiciono la cámara en el eje z -3 y luego voy a invertir su dirección
	view[3].z = -3; // Esto es un acceso de la matriz, es una forma de acceder a la matriz, columna 4 fila 4 (el array empieza en 0)

	// Con esto tendría suficiente para una matriz de vista, ahora hago una matriz de proyección
	glm::mat4 proj = glm::mat4(1.0);

	// Construyo f, near y far
	float f = 1.0 / tan(M_PI / 6.0);
	float far = 10.0; // Plano lejano
	float near = 0.1; // Plano cercano

	// Cojo elemento a elemento de la matriz de proyección y voy a ir tocandolos uno a uno

	proj[0].x = f; // Primera columna del elemento de la matriz
	proj[1].y = f; // El uno a uno de la diagonal de la matriz
	proj[2].z = (far + near) / (near - far);
	proj[2].w = -1.0;
	proj[3].z = (2.0 * far * near) / (near - far);
	proj[3].w = 0.0;


	
	IGlib::setProjMat(proj); 
	IGlib::setViewMat(view);
	
	if (!DoTheImportThing("../3D_assets/AlienAnimal.obj")) {
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

	//CBs (Callbacks)
	// Cada vez que el sistema detecte que el usuario ha rescalado la pantalla, hecho click, etc. Recoge los eventos y nos avisa
	IGlib::setResizeCB(resizeFunc);
	IGlib::setIdleCB(idleFunc);
	IGlib::setKeyboardCB(keyboardFunc);
	IGlib::setMouseCB(mouseFunc);
	IGlib::setMouseMoveCB(mouseMoveFunc);
	IGlib::setMousePassiveMoveCB(mousePassMoveFunc);
	IGlib::setMouseWheelMoveCB(mouseWheelFunc);


	//Mainloop
	IGlib::mainLoop();
	IGlib::destroy();
	return 0;
}

void resizeFunc(int width, int height)
{
	//Ajusta el aspect ratio al tama�o de la venta
}

void idleFunc()
{
	static float angle = 0.0f;

	angle = (angle < M_PI * 2.0) ? angle + 0.001 : 0.0f; // Si el ángulo es más de 60 grados, lo amplío, y sino reseteo 0.0. Hará que esté continuamente en rotación

	glm::mat4 modelMat = glm::translate(glm::mat4(1.0f),glm::vec3(0.0f,0.0f,-6.0f)); // Construye una matriz de translación con lo que yo le diga
	modelMat = glm::rotate(modelMat, angle, glm::vec3(1.0f, 0.0f, 0.0f)); // Yo altero la modelMatrix del cubo
	modelMat = glm::rotate(modelMat, angle, glm::vec3(0.0f, 1.0f, 0.0f)); // El orden de las matrices importan mucho en el movimiento!!!
	IGlib::setModelMat(objId, modelMat);
}

void keyboardFunc(unsigned char key, int x, int y)
{
	std::cout << "Se ha pulsado la tecla " << key << std::endl << std::endl;
}

void mouseFunc(int button, int state, int x, int y)
{

	if (state == 0)
		std::cout << "Se ha pulsado el bot�n ";
	else
		std::cout << "Se ha soltado el bot�n ";

	if (button == 0) std::cout << "de la izquierda del rat�n " << std::endl;
	if (button == 1) std::cout << "central del rat�n " << std::endl;
	if (button == 2) std::cout << "de la derecha del rat�n " << std::endl;


	std::cout << "en la posici�n " << x << " " << y << std::endl << std::endl;
}


void mouseMoveFunc(int x, int y)
{

	//std::cout << "Movimiento con presi�n en la posici�n " << x << " " << y << std::endl << std::endl; 
}

void mousePassMoveFunc(int x, int y)
{
	//std::cout << "Pasivo en la posici�n " << x << " " << y << std::endl << std::endl;
}
void mouseWheelFunc(int wheel, int dir, int x, int y)
{
	if (dir > 0)
		std::cout << "Se ha pulsado la rueda del rat�n hacia arriba ";
	else
		std::cout << "Se ha pulsado la rueda del rat�n hacia abajo ";

	std::cout << "en la posici�n " << x << " " << y << std::endl << std::endl;
}

bool DoTheImportThing(const std::string& pFile) {
	
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

		if (mesh->HasFaces()) {//por defecto siempre tiene, pero no está de más chequear
				for (unsigned int t = 0; t < mesh->mNumFaces; ++t) {
					const aiFace* face = &mesh->mFaces[t];
					memcpy(&faceArray[faceIndex], face->mIndices, 3 *
						sizeof(unsigned int));
					faceIndex += 3;
				}
		}
		/*
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
		*/
		objId = IGlib::createObj(mesh->mNumFaces, mesh->mNumVertices, faceArray, vertexArray, 0, normalArray, textCoordsArray);
		glm::mat4 modelMat = glm::scale(glm::mat4(1.0), glm::vec3(0.2));
		IGlib::setModelMat(objId, modelMat);
		IGlib::addColorTex(objId, "../3D_assets/Alien-Animal-Base-Diffuse.jpg");
	}

	return true;
}