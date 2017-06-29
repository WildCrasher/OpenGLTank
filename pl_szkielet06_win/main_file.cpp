/*
Niniejszy program jest wolnym oprogramowaniem; mo�esz go
rozprowadza� dalej i / lub modyfikowa� na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundacj� Wolnego
Oprogramowania - wed�ug wersji 2 tej Licencji lub(wed�ug twojego
wyboru) kt�rej� z p�niejszych wersji.

Niniejszy program rozpowszechniany jest z nadziej�, i� b�dzie on
u�yteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domy�lnej
gwarancji PRZYDATNO�CI HANDLOWEJ albo PRZYDATNO�CI DO OKRE�LONYCH
ZASTOSOWA�.W celu uzyskania bli�szych informacji si�gnij do
Powszechnej Licencji Publicznej GNU.

Z pewno�ci� wraz z niniejszym programem otrzyma�e� te� egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
je�li nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/

#define GLM_FORCE_RADIANS

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "constants.h"
#include "allmodels.h"
#include "lodepng.h"
#include "shaderprogram.h"

using namespace glm;

float speed_forward = 0;
float speed_side = 0;
float speed_round_turret = 0;

//Uchwyty na shadery
ShaderProgram *shaderProgram;


GLuint vao_hull;
GLuint bufHullVertices;
//GLuint bufColors;
GLuint bufHullNormals;
GLuint bufHullTexCoords;

GLuint vao_turret;
GLuint bufTurretVertices;
//GLuint bufColors;
GLuint bufTurretNormals;
GLuint bufTurretTexCoords;

GLuint vao_gun;
GLuint bufGunVertices;
//GLuint bufColors;
GLuint bufGunNormals;
GLuint bufGunTexCoords;

//HULL
float* hullvertices=Models::TankHullInternal::hullPositions;
//float* colors=Models::CubeInternal::colors;
float* hullnormals=Models::TankHullInternal::hullNormals;
float* hulltexCoords=Models::TankHullInternal::hullTexels;
//float* normals=Models::CubeInternal::vertexNormals;
int hullvertexCount=Models::TankHullInternal::hullVertices;


//TURRET
float* turretvertices=Models::TankTurretInternal::turretPositions;
//float* colors=Models::CubeInternal::colors;
float* turretnormals=Models::TankTurretInternal::turretNormals;
float* turrettexCoords=Models::TankTurretInternal::turretTexels;
//float* normals=Models::CubeInternal::vertexNormals;
int turretvertexCount=Models::TankTurretInternal::turretVertices;

float* gunvertices=Models::TankGunInternal::gunPositions;
//float* colors=Models::CubeInternal::colors;
float* gunnormals=Models::TankGunInternal::gunNormals;
float* guntexCoords=Models::TankGunInternal::gunTexels;
//float* normals=Models::CubeInternal::vertexNormals;
int gunvertexCount=Models::TankGunInternal::gunVertices;


//Uchwyty na tekstury
GLuint tex0;
GLuint tex1;

GLuint readTexture(const char* filename) {
	GLuint tex;
	glActiveTexture(GL_TEXTURE0);

	//Wczytanie do pami�ci komputera
	std::vector<unsigned char> image;   //Alokuj wektor do wczytania obrazka
	unsigned width, height;   //Zmienne do kt�rych wczytamy wymiary obrazka
	//Wczytaj obrazek
	unsigned error = lodepng::decode(image, width, height, filename);

	if (error!=0) {
        printf("Error while reading texture %s. Error code: %d. \n",filename,error);
	}

	//Import do pami�ci karty graficznej
	glGenTextures(1,&tex); //Zainicjuj jeden uchwyt
	glBindTexture(GL_TEXTURE_2D, tex); //Uaktywnij uchwyt
	//Wczytaj obrazek do pami�ci KG skojarzonej z uchwytem
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
	GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*) image.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	return tex;
}

//Procedura obs�ugi b��d�w
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

//Procedura obs�ugi klawiatury
void key_callback(GLFWwindow* window, int key,
	int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_LEFT) speed_side = 1;
		if (key == GLFW_KEY_RIGHT) speed_side = -1;
		if (key == GLFW_KEY_UP) speed_forward = -0.3;
		if (key == GLFW_KEY_DOWN) speed_forward = 0.3;
		if (key == GLFW_KEY_A) speed_round_turret = -1.5;
		if (key == GLFW_KEY_D) speed_round_turret = 1.5;
	}


	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_LEFT) speed_side = 0;
		if (key == GLFW_KEY_RIGHT) speed_side = 0;
		if (key == GLFW_KEY_UP) speed_forward = 0;
		if (key == GLFW_KEY_DOWN) speed_forward = 0;
		if (key == GLFW_KEY_A) speed_round_turret = 0;
		if (key == GLFW_KEY_D) speed_round_turret = 0;
	}
}

//Tworzy bufor VBO z tablicy
GLuint makeBuffer(void *data, int vertexCount, int vertexSize) {
	GLuint handle;

	glGenBuffers(1,&handle);//Wygeneruj uchwyt na Vertex Buffer Object (VBO), kt�ry b�dzie zawiera� tablic� danych
	glBindBuffer(GL_ARRAY_BUFFER,handle);  //Uaktywnij wygenerowany uchwyt VBO
	glBufferData(GL_ARRAY_BUFFER, vertexCount*vertexSize, data, GL_STATIC_DRAW);//Wgraj tablic� do VBO

	return handle;
}

//Przypisuje bufor VBO do atrybutu
void assignVBOtoAttribute(ShaderProgram *shaderProgram,const char* attributeName, GLuint bufVBO, int vertexSize) {
	GLuint location=shaderProgram->getAttribLocation(attributeName); //Pobierz numery slot�w dla atrybutu
	glBindBuffer(GL_ARRAY_BUFFER,bufVBO);  //Uaktywnij uchwyt VBO
	glEnableVertexAttribArray(location); //W��cz u�ywanie atrybutu o numerze slotu zapisanym w zmiennej location
	glVertexAttribPointer(location,vertexSize,GL_FLOAT, GL_FALSE, 0, NULL); //Dane do slotu location maj� by� brane z aktywnego VBO
}


//Procedura inicjuj�ca
void initOpenGLProgram(GLFWwindow* window) {
	//************Tutaj umieszczaj kod, kt�ry nale�y wykona� raz, na pocz�tku programu************
	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glfwSetKeyCallback(window, key_callback);

	shaderProgram=new ShaderProgram("vshader.txt",NULL,"fshader.txt");

	//tworzenie bufora HULL
	bufHullVertices=makeBuffer(hullvertices, hullvertexCount, sizeof(float)*4);
	//bufColors=makeBuffer(colors, vertexCount, sizeof(float)*4);
	bufHullNormals=makeBuffer(hullnormals, hullvertexCount, sizeof(float)*4);
	bufHullTexCoords=makeBuffer(hulltexCoords, hullvertexCount, sizeof(float)*2);

    glGenVertexArrays(1,&vao_hull);

    glBindVertexArray(vao_hull);

    assignVBOtoAttribute(shaderProgram,"vertex",bufHullVertices,4);
	//assignVBOtoAttribute(shaderProgram,"color",bufColors,4);
	assignVBOtoAttribute(shaderProgram,"normal",bufHullNormals,4);
	assignVBOtoAttribute(shaderProgram,"texCoords",bufHullTexCoords,2);

	//tworzenie bufora TURRET
	bufTurretVertices=makeBuffer(turretvertices, turretvertexCount, sizeof(float)*4);
	//bufColors=makeBuffer(colors, vertexCount, sizeof(float)*4);
	bufTurretNormals=makeBuffer(turretnormals, turretvertexCount, sizeof(float)*4);
	bufTurretTexCoords=makeBuffer(turrettexCoords, turretvertexCount, sizeof(float)*2);

    glGenVertexArrays(1,&vao_turret);

	glBindVertexArray(vao_turret);

	assignVBOtoAttribute(shaderProgram,"vertex",bufTurretVertices,4);
	//assignVBOtoAttribute(shaderProgram,"color",bufColors,4);
	assignVBOtoAttribute(shaderProgram,"normal",bufTurretNormals,4);
	assignVBOtoAttribute(shaderProgram,"texCoords",bufTurretTexCoords,2);

	//tworzenie bufora GUN
	bufGunVertices=makeBuffer(gunvertices, gunvertexCount, sizeof(float)*4);
	//bufColors=makeBuffer(colors, vertexCount, sizeof(float)*4);
	bufGunNormals=makeBuffer(gunnormals, gunvertexCount, sizeof(float)*4);
	bufGunTexCoords=makeBuffer(guntexCoords, gunvertexCount, sizeof(float)*2);

    glGenVertexArrays(1,&vao_gun);

    glBindVertexArray(vao_gun);

    assignVBOtoAttribute(shaderProgram,"vertex",bufGunVertices,4);
	//assignVBOtoAttribute(shaderProgram,"color",bufColors,4);
	assignVBOtoAttribute(shaderProgram,"normal",bufGunNormals,4);
	assignVBOtoAttribute(shaderProgram,"texCoords",bufGunTexCoords,2);

	glBindVertexArray(0);

	tex0=readTexture("metal.png");
	tex1=readTexture("metal_spec.png");
}

void freeOpenGLProgram() {
	delete shaderProgram;

	glDeleteVertexArrays(1,&vao_hull);
	glDeleteBuffers(1,&bufHullVertices);
	//glDeleteBuffers(1,&bufColors);
	glDeleteBuffers(1,&bufHullNormals);
	glDeleteBuffers(1,&bufHullTexCoords);


	glDeleteVertexArrays(1,&vao_turret);
	glDeleteBuffers(1,&bufTurretVertices);
	//glDeleteBuffers(1,&bufColors);
	glDeleteBuffers(1,&bufTurretNormals);
	glDeleteBuffers(1,&bufTurretTexCoords);

	glDeleteVertexArrays(1,&vao_gun);
	glDeleteBuffers(1,&bufGunVertices);
	//glDeleteBuffers(1,&bufColors);
	glDeleteBuffers(1,&bufGunNormals);
	glDeleteBuffers(1,&bufGunTexCoords);

	glDeleteTextures(1,&tex0);
	glDeleteTextures(1,&tex1);

}

void drawObject(GLuint vao, ShaderProgram *shaderProgram, mat4 mP, mat4 mV, mat4 mM, int vertexCount) {
	//W��czenie programu cieniuj�cego, kt�ry ma zosta� u�yty do rysowania
	//W tym programie wystarczy�oby wywo�a� to raz, w setupShaders, ale chodzi o pokazanie,
	//�e mozna zmienia� program cieniuj�cy podczas rysowania jednej sceny
	shaderProgram->use();

	//Przeka� do shadera macierze P,V i M.
	//W linijkach poni�ej, polecenie:
	//  shaderProgram->getUniformLocation("P")
	//pobiera numer slotu odpowiadaj�cego zmiennej jednorodnej o podanej nazwie
	//UWAGA! "P" w powy�szym poleceniu odpowiada deklaracji "uniform mat4 P;" w vertex shaderze,
	//a mP w glm::value_ptr(mP) odpowiada argumentowi  "mat4 mP;" TYM pliku.
	//Ca�a poni�sza linijka przekazuje do zmiennej jednorodnej P w vertex shaderze dane z argumentu mP niniejszej funkcji
	//Pozosta�e polecenia dzia�aj� podobnie.
	glUniformMatrix4fv(shaderProgram->getUniformLocation("P"),1, false, glm::value_ptr(mP));
	glUniformMatrix4fv(shaderProgram->getUniformLocation("V"),1, false, glm::value_ptr(mV));
	glUniformMatrix4fv(shaderProgram->getUniformLocation("M"),1, false, glm::value_ptr(mM));
	//Przeka� wsp�rz�dne �wiat�a do zmiennej jednorodnej "lp". Obydwie poni�sze linijki dzia�aj� tak samo
	glUniform4f(shaderProgram->getUniformLocation("lp"),0,0,-6,1); //Kolejne argumenty to kolejne warto�ci wektora
	//glUniform4fv(shaderProgram->getUniformLocation("lp"),1,value_ptr(vec4(0,0,-6,1))); //drugi argument - przesylamy tylko jeden wektor, trzeci argument - wektor do przes�ania

	//Przypisanie jednostek teksturuj�cych do zmiennych
	glUniform1i(shaderProgram->getUniformLocation("textureMap0"),0);
	glUniform1i(shaderProgram->getUniformLocation("textureMap1"),1);

	//Przypisanie tekstur do jednostek teksturuj�cych
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,tex0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,tex1);

	//Uaktywnienie VAO i tym samym uaktywnienie predefiniowanych w tym VAO powi�za� slot�w atrybut�w z tablicami z danymi
	glBindVertexArray(vao);

	//Narysowanie obiektu
	glDrawArrays(GL_TRIANGLES,0,vertexCount);

	//Posprz�tanie po sobie (niekonieczne w sumie je�eli korzystamy z VAO dla ka�dego rysowanego obiektu)
	glBindVertexArray(0);
}

void srednia_x_y_z( const float* vertices, const int vertexCount, float srednie[3])
{
    float maks[3];
    float minn[3];

    maks[0] = vertices[0];
    maks[1] = vertices[1];
    maks[2] = vertices[2];

    for(int i = 0; i < vertexCount; i = i + 4 )
    {
        if( vertices[i] > maks[0] ) maks[0] = vertices[i];
        if( vertices[i + 1] > maks[1] ) maks[1] = vertices[i + 1];
        if( vertices[i + 2] > maks[2] ) maks[2] = vertices[i + 2];
    }

    minn[0] = vertices[0];
    minn[1] = vertices[1];
    minn[2] = vertices[2];

    for(int i = 0; i < turretvertexCount; i = i + 4 )
    {
        if( vertices[i] < minn[0] ) minn[0] = vertices[i];
        if( vertices[i + 1] < minn[1] ) minn[1] = vertices[i + 1];
        if( vertices[i + 2] < minn[2] ) minn[2] = vertices[i + 2];
    }
    srednie[0] = (maks[0] + minn[0])/2.0f;
    srednie[1] = (maks[1] + minn[1])/2.0f;
    srednie[2] = (maks[2] + minn[2])/2.0f;
}

//Procedura rysuj�ca zawarto�� sceny
void drawScene(GLFWwindow* window, float move_x,float move_z, float move_side, float move_round_turret) {
	//************Tutaj umieszczaj kod rysuj�cy obraz******************l

    float srednie_hull[3];
    float srednie_turret[3];

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); //Wykonaj czyszczenie bufora kolor�w

	glm::mat4 P = glm::perspective(50 * PI / 180, 1.0f, 1.0f, 50.0f); //Wylicz macierz rzutowania

	glm::mat4 V = glm::lookAt( //Wylicz macierz widoku
		glm::vec3(0.0f, 30.0f, -30.5f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));


	//HUll
	srednia_x_y_z(hullvertices, hullvertexCount, srednie_hull);
    srednia_x_y_z(turretvertices, turretvertexCount, srednie_turret);
    srednie_turret[0] = srednie_turret[0] - 1; //dostosowanie osi obrotu
    srednie_turret[2] = srednie_turret[2] - 1;

	glm::mat4 M_Hull = glm::mat4(1.0f);
	M_Hull = glm::translate(M_Hull, glm::vec3(move_z, 0, move_x));
	M_Hull = glm::translate(M_Hull, glm::vec3(  srednie_hull[0], srednie_hull[1], srednie_hull[2]));
	M_Hull = glm::rotate(M_Hull, move_side, glm::vec3(0, 1, 0));
    M_Hull = glm::translate(M_Hull, glm::vec3(  -srednie_hull[0], -srednie_hull[1], -srednie_hull[2]));

    //TURRET
	glm::mat4 M_Turret = glm::mat4(1.0f);
    M_Turret = glm::translate(M_Turret, glm::vec3(move_z, 0, move_x));
    M_Turret = glm::translate(M_Turret, glm::vec3(  srednie_hull[0], srednie_hull[1], srednie_hull[2]));
	M_Turret = glm::rotate(M_Turret, move_side, glm::vec3(0, 1, 0));
    M_Turret = glm::translate(M_Turret, glm::vec3(  -srednie_hull[0], -srednie_hull[1], -srednie_hull[2]));

    M_Turret = glm::translate(M_Turret, glm::vec3(  srednie_turret[0], srednie_turret[1], srednie_turret[2]));
	M_Turret = glm::rotate(M_Turret, move_round_turret, glm::vec3(0, 1, 0));
    M_Turret = glm::translate(M_Turret, glm::vec3( -srednie_turret[0], -srednie_turret[1], -srednie_turret[2]));

    std::cout<<srednie_turret[0]<<" "<<srednie_turret[1]<<" "<<srednie_turret[2]<<std::endl;
    //GUN
	glm::mat4 M_Gun = glm::mat4(1.0f);
	M_Gun = glm::translate(M_Gun, glm::vec3(move_z, 0, move_x));
    M_Gun = glm::translate(M_Gun, glm::vec3(  srednie_hull[0], srednie_hull[1], srednie_hull[2]));
	M_Gun = glm::rotate(M_Gun, move_side, glm::vec3(0, 1, 0));
    M_Gun = glm::translate(M_Gun, glm::vec3(  -srednie_hull[0], -srednie_hull[1], -srednie_hull[2]));

	M_Gun = glm::translate(M_Gun, glm::vec3(  srednie_turret[0], srednie_turret[1], srednie_turret[2]));
	M_Gun = glm::rotate(M_Gun, move_round_turret, glm::vec3(0, 1, 0));
    M_Gun = glm::translate(M_Gun, glm::vec3( -srednie_turret[0], -srednie_turret[1], -srednie_turret[2]));

	//Narysuj obiekt
	drawObject(vao_hull,shaderProgram,P,V,M_Hull, hullvertexCount);
	drawObject(vao_turret,shaderProgram,P,V,M_Turret, turretvertexCount);
	drawObject(vao_gun,shaderProgram,P,V,M_Gun, gunvertexCount);

	//Przerzu� tylny bufor na przedni
	glfwSwapBuffers(window);

}


int main(void)
{
	GLFWwindow* window; //Wska�nik na obiekt reprezentuj�cy okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedur� obs�ugi b��d�w

	if (!glfwInit()) { //Zainicjuj bibliotek� GLFW
		fprintf(stderr, "Nie mo�na zainicjowa� GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(800, 800, "Tank", NULL, NULL);  //Utw�rz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

	if (!window) //Je�eli okna nie uda�o si� utworzy�, to zamknij program
	{
		fprintf(stderr, "Nie mo�na utworzy� okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje si� aktywny i polecenia OpenGL b�d� dotyczy� w�a�nie jego.
	glfwSwapInterval(1); //Czekaj na 1 powr�t plamki przed pokazaniem ukrytego bufora

	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotek� GLEW
		fprintf(stderr, "Nie mo�na zainicjowa� GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Operacje inicjuj�ce

	float move_x = 0; //K�t obrotu obiektu
	float move_z = 0;
	float move_side = 0; //K�t obrotu obiektu
	float move_round_turret = 0;

	glfwSetTime(0); //Wyzeruj licznik czasu

	while (!glfwWindowShouldClose(window))
	{
		move_x += speed_forward*sin(move_side);
		move_z += speed_forward*(-cos(move_side));
		move_side += speed_side*glfwGetTime();
		move_round_turret += speed_round_turret*glfwGetTime();
		glfwSetTime(0);
		drawScene(window, move_x, move_z, move_side, move_round_turret);
		glfwPollEvents();
	}

	freeOpenGLProgram();

	glfwDestroyWindow(window); //Usu� kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zaj�te przez GLFW
	exit(EXIT_SUCCESS);
}
