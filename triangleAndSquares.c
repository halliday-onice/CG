//*****************************************************
//
// triangleAndSquares.cpp
// Um programa OpenGL simples que abre uma janela GLUT
// e desenha um tri�ngulo no centro
//
// Marcelo Cohen e Isabel H. Manssour
// Este c�digo acompanha o livro 
// "OpenGL - Uma Abordagem Pr�tica e Objetiva"
//  
// 
//*****************************************************
#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>
float angulo;

typedef struct {
	float x;
	float y;
	float z;
} centroide;

// Fun��o callback de redesenho da janela de visualiza��o



// eu preciso primeiro descobrir a coordenada do ponto de cima para poder seguir a ideia de rotacionar, pra poder fazer a funcao abaixo
//o melhor pelo que entendi eh ter o left bottom do triangulo na origem


// Os proximos passos sao: colocar os quadrados e rotaciona-los
// 
// 
float distance(float x1, float y1, float x2, float y2) {
	return sqrtf((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
}
  
void CalculateTriangleSides() {
	// Posições dos pontos
	float ax = 0.0f, ay = 0.0f;
	float bx = 1.0f, by = 0.0f;
	float cx = 0.5f, cy = sqrtf(3)/2;
  
	float AB = distance(ax, ay, bx, by);
	float BC = distance(bx, by, cx, cy);
	float CA = distance(cx, cy, ax, ay);
  
	printf("Lado AB: %.3f\n", AB);
	printf("Lado BC: %.3f\n", BC);
	printf("Lado CA: %.3f\n", CA);
}
void CalculateUpperDotTriangle(float ax, float ay, float bx, float by){

	// // Calcula ponto do meio da base
	float middleX = (ax + bx)/ 2.0f;
	float middleY = (bx + by) / 2.0f;

	//vetor perpendicular a base
	float perpX = -(by -ay);
	float perpY = bx - ax;

	// Normaliza
	float length = sqrt(perpX * perpX + perpY * perpY);
	perpX /= length;
	perpY /= length;
	
	float height = sqrt(3)/2.0; // altura de um triângulo equilátero com lado 

	// Coordenadas do ponto superior (cx, cy)
	float cx = middleX + perpX * height;
	float cy = middleY + perpY * height;

    
    
	glBegin(GL_TRIANGLES);
		glVertex3f(ax, ay, 0);
		glVertex3f(bx, by, 0);
		glVertex3f(cx, cy, 0);
	glEnd();
    
}

void DesenhaTriangulo(){
    glClearColor(1,1,1,0);
	glClear(GL_COLOR_BUFFER_BIT);

	// Define a cor de desenho: vermelho
	glColor3f(0,0,1);
    
	glPushMatrix();
		// Use a função com dois pontos da base do triângulo
		float ax = 0.0f, ay = 0.0f;
		float bx = 1.0f, by = 0.0f;
	
		CalculateUpperDotTriangle(ax,ay,bx, by);

        

    	glPopMatrix();
    	glFlush();

}





void DesenhaQuadrado(void)
{
	// Limpa a janela de visualiza��o com a cor branca
	glClearColor(1,1,1,0);
	glClear(GL_COLOR_BUFFER_BIT);

	// Define a cor de desenho: vermelho
	glColor3f(0,0,1);
	
	//scale function always comes first
	//glScalef(0.5, 0.5, 0.5); // o quadrado eh metade da viewport
	glRotatef(angulo, 0 ,0,1);
	//glTranslatef()

	// Desenha um quadrado
	glBegin(GL_QUADS);
		glVertex3f(-0.75,-0.75,0); //left bottom
		glVertex3f(0.75,-0.75,0); //right bottom
		glVertex3f(0.75,0.75,0); //top right
		glVertex3f(-0.75,0.75,0); //top left
	glEnd();

    
	
	//Executa os comandos OpenGL 
	glFlush();
}

// Fun��o callback chamada para gerenciar eventos de teclas
void Teclado (unsigned char key, int x, int y)
{
	if (key == 27)
		exit(0);
	if (key == 32)
		angulo += 10;
		glutPostRedisplay();
}

// Fun��o respons�vel por inicializar par�metros e vari�veis
void Inicializa(void)
{
	// Define a janela de visualiza��o 2D
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(-1.0,1.0,-1.0,1.0);
	glMatrixMode(GL_MODELVIEW);
}

// Programa Principal 
int main(int argc, char **argv)
{
	CalculateTriangleSides();
	glutInit(&argc, argv);
	// Define do modo de opera��o da GLUT
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB); 
 
	// Especifica o tamanho inicial em pixels da janela GLUT
	glutInitWindowSize(600,600); 
 
	// Cria a janela passando como argumento o t�tulo da mesma
	glutCreateWindow("Primeiro Programa");
 
	// Registra a fun��o callback de redesenho da janela de visualiza��o
	//glutDisplayFunc(DesenhaQuadrado);
    	glutDisplayFunc(DesenhaTriangulo);

	// Registra a fun��o callback para tratamento das teclas ASCII
	glutKeyboardFunc (Teclado);

	// Chama a fun��o respons�vel por fazer as inicializa��es 
	Inicializa();
 
	// Inicia o processamento e aguarda intera��es do usu�rio
	glutMainLoop();
	printf("teste\n");
 
	return 0;
}
