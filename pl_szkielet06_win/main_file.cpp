/*
Niniejszy program jest wolnym oprogramowaniem; mo¿esz go
rozprowadzaæ dalej i / lub modyfikowaæ na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundacjê Wolnego
Oprogramowania - wed³ug wersji 2 tej Licencji lub(wed³ug twojego
wyboru) którejœ z póŸniejszych wersji.

Niniejszy program rozpowszechniany jest z nadziej¹, i¿ bêdzie on
u¿yteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyœlnej
gwarancji PRZYDATNOŒCI HANDLOWEJ albo PRZYDATNOŒCI DO OKREŒLONYCH
ZASTOSOWAÑ.W celu uzyskania bli¿szych informacji siêgnij do
Powszechnej Licencji Publicznej GNU.

Z pewnoœci¹ wraz z niniejszym programem otrzyma³eœ te¿ egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
jeœli nie - napisz do Free Software Foundation, Inc., 59 Temple
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
#include "constants.h"
#include "allmodels.h"
#include "lodepng.h"
#include "shaderprogram.h"

using namespace glm;

float speed_forward = 0; // [radiany/s]
float speed_side = 0; // [radiany/s]

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

	//Wczytanie do pamiêci komputera
	std::vector<unsigned char> image;   //Alokuj wektor do wczytania obrazka
	unsigned width, height;   //Zmienne do których wczytamy wymiary obrazka
	//Wczytaj obrazek
	unsigned error = lodepng::decode(image, width, height, filename);

	if (error!=0) {
        printf("Error while reading texture %s. Error code: %d. \n",filename,error);
	}

	//Import do pamiêci karty graficznej
	glGenTextures(1,&tex); //Zainicjuj jeden uchwyt
	glBindTexture(GL_TEXTURE_2D, tex); //Uaktywnij uchwyt
	//Wczytaj obrazek do pamiêci KG skojarzonej z uchwytem
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
	GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*) image.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	return tex;
}

//Procedura obs³ugi b³êdów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

//Procedura obs³ugi klawiatury
void key_callback(GLFWwindow* window, int key,
	int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_LEFT) speed_side = -0.5;
		if (key == GLFW_KEY_RIGHT) speed_side = 0.5;
		if (key == GLFW_KEY_UP) speed_forward = -6;
		if (key == GLFW_KEY_DOWN) speed_forward = 6;
	}


	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_LEFT) speed_side = 0;
		if (key == GLFW_KEY_RIGHT) speed_side = 0;
		if (key == GLFW_KEY_UP) speed_forward = 0;
		if (key == GLFW_KEY_DOWN) speed_forward = 0;
	}
}

//Tworzy bufor VBO z tablicy
GLuint makeBuffer(void *data, int vertexCount, int vertexSize) {
	GLuint handle;

	glGenBuffers(1,&handle);//Wygeneruj uchwyt na Vertex Buffer Object (VBO), który bêdzie zawiera³ tablicê danych
	glBindBuffer(GL_ARRAY_BUFFER,handle);  //Uaktywnij wygenerowany uchwyt VBO
	glBufferData(GL_ARRAY_BUFFER, vertexCount*vertexSize, data, GL_STATIC_DRAW);//Wgraj tablicê do VBO

	return handle;
}

//Przypisuje bufor VBO do atrybutu
void assignVBOtoAttribute(ShaderProgram *shaderProgram,const char* attributeName, GLuint bufVBO, int vertexSize) {
	GLuint location=shaderProgram->getAttribLocation(attributeName); //Pobierz numery slotów dla atrybutu
	glBindBuffer(GL_ARRAY_BUFFER,bufVBO);  //Uaktywnij uchwyt VBO
	glEnableVertexAttribArray(location); //W³¹cz u¿ywanie atrybutu o numerze slotu zapisanym w zmiennej location
	glVertexAttribPointer(location,vertexSize,GL_FLOAT, GL_FALSE, 0, NULL); //Dane do slotu location maj¹ byæ brane z aktywnego VBO
}


//Procedura inicjuj¹ca
void initOpenGLProgram(GLFWwindow* window) {
	//************Tutaj umieszczaj kod, który nale¿y wykonaæ raz, na pocz¹tku programu************
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
	//W³¹czenie programu cieniuj¹cego, który ma zostaæ u¿yty do rysowania
	//W tym programie wystarczy³oby wywo³aæ to raz, w setupShaders, ale chodzi o pokazanie,
	//¿e mozna zmieniaæ program cieniuj¹cy podczas rysowania jednej sceny
	shaderProgram->use();

	//Przeka¿ do shadera macierze P,V i M.
	//W linijkach poni¿ej, polecenie:
	//  shaderProgram->getUniformLocation("P")
	//pobiera numer slotu odpowiadaj¹cego zmiennej jednorodnej o podanej nazwie
	//UWAGA! "P" w powy¿szym poleceniu odpowiada deklaracji "uniform mat4 P;" w vertex shaderze,
	//a mP w glm::value_ptr(mP) odpowiada argumentowi  "mat4 mP;" TYM pliku.
	//Ca³a poni¿sza linijka przekazuje do zmiennej jednorodnej P w vertex shaderze dane z argumentu mP niniejszej funkcji
	//Pozosta³e polecenia dzia³aj¹ podobnie.
	glUniformMatrix4fv(shaderProgram->getUniformLocation("P"),1, false, glm::value_ptr(mP));
	glUniformMatrix4fv(shaderProgram->getUniformLocation("V"),1, false, glm::value_ptr(mV));
	glUniformMatrix4fv(shaderProgram->getUniformLocation("M"),1, false, glm::value_ptr(mM));
	//Przeka¿ wspó³rzêdne œwiat³a do zmiennej jednorodnej "lp". Obydwie poni¿sze linijki dzia³aj¹ tak samo
	glUniform4f(shaderProgram->getUniformLocation("lp"),0,0,-6,1); //Kolejne argumenty to kolejne wartoœci wektora
	//glUniform4fv(shaderProgram->getUniformLocation("lp"),1,value_ptr(vec4(0,0,-6,1))); //drugi argument - przesylamy tylko jeden wektor, trzeci argument - wektor do przes³ania

	//Przypisanie jednostek teksturuj¹cych do zmiennych
	glUniform1i(shaderProgram->getUniformLocation("textureMap0"),0);
	glUniform1i(shaderProgram->getUniformLocation("textureMap1"),1);

	//Przypisanie tekstur do jednostek teksturuj¹cych
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,tex0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,tex1);

	//Uaktywnienie VAO i tym samym uaktywnienie predefiniowanych w tym VAO powi¹zañ slotów atrybutów z tablicami z danymi
	glBindVertexArray(vao);

	//Narysowanie obiektu
	glDrawArrays(GL_TRIANGLES,0,vertexCount);

	//Posprz¹tanie po sobie (niekonieczne w sumie je¿eli korzystamy z VAO dla ka¿dego rysowanego obiektu)
	glBindVertexArray(0);
}

//Procedura rysuj¹ca zawartoœæ sceny
void drawScene(GLFWwindow* window, float move_forward, float move_side) {
	//************Tutaj umieszczaj kod rysuj¹cy obraz******************l

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); //Wykonaj czyszczenie bufora kolorów

	glm::mat4 P = glm::perspective(50 * PI / 180, 1.0f, 1.0f, 50.0f); //Wylicz macierz rzutowania

	glm::mat4 V = glm::lookAt( //Wylicz macierz widoku
		glm::vec3(0.0f, 0.0f, -30.5f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));


	//Wylicz macierz modelu rysowanego obiektu
	glm::mat4 M = glm::mat4(1.0f);
	M = glm::translate(M, glm::vec3(move_forward, 0, 0));
	M = glm::rotate(M, move_side, glm::vec3(0, 1, 0));

	//Narysuj obiekt
	drawObject(vao_hull,shaderProgram,P,V,M, hullvertexCount);
	drawObject(vao_turret,shaderProgram,P,V,M, turretvertexCount);
	drawObject(vao_gun,shaderProgram,P,V,M, gunvertexCount);

	//Przerzuæ tylny bufor na przedni
	glfwSwapBuffers(window);

}


int main(void)
{
	GLFWwindow* window; //WskaŸnik na obiekt reprezentuj¹cy okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedurê obs³ugi b³êdów

	if (!glfwInit()) { //Zainicjuj bibliotekê GLFW
		fprintf(stderr, "Nie mo¿na zainicjowaæ GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(500, 500, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

	if (!window) //Je¿eli okna nie uda³o siê utworzyæ, to zamknij program
	{
		fprintf(stderr, "Nie mo¿na utworzyæ okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje siê aktywny i polecenia OpenGL bêd¹ dotyczyæ w³aœnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekê GLEW
		fprintf(stderr, "Nie mo¿na zainicjowaæ GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Operacje inicjuj¹ce

	float move_forward = 0; //K¹t obrotu obiektu
	float move_side = 0; //K¹t obrotu obiektu

	glfwSetTime(0); //Wyzeruj licznik czasu

	//G³ówna pêtla
	while (!glfwWindowShouldClose(window)) //Tak d³ugo jak okno nie powinno zostaæ zamkniête
	{
		move_forward += speed_forward*glfwGetTime(); //Zwiêksz k¹t o prêdkoœæ k¹tow¹ razy czas jaki up³yn¹³ od poprzedniej klatki
		move_side += speed_side*glfwGetTime(); //Zwiêksz k¹t o prêdkoœæ k¹tow¹ razy czas jaki up³yn¹³ od poprzedniej klatki
		glfwSetTime(0); //Wyzeruj licznik czasu
		drawScene(window,move_forward,move_side); //Wykonaj procedurê rysuj¹c¹
		glfwPollEvents(); //Wykonaj procedury callback w zaleznoœci od zdarzeñ jakie zasz³y.
	}

	freeOpenGLProgram();

	glfwDestroyWindow(window); //Usuñ kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajête przez GLFW
	exit(EXIT_SUCCESS);
}
