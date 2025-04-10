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




// eu preciso primeiro descobrir a coordenada do ponto de cima para poder seguir a ideia de rotacionar, pra poder fazer a funcao abaixo
//o melhor pelo que entendi eh ter o left bottom do triangulo na origem


//
// 
// distance e calculateTriangleSides sao funcoes axiliares
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
//
//
//
void DesenhaQuadradoNaAresta(float x1, float y1, float x2, float y2, int inverted) {
	// Vetor da base do quadrado (x2 - x1, y2 - y1)
	
	float dx = x2 - x1;
	float dy = y2 - y1;
  
	// Vetor perpendicular (90 graus anti-horário)
	float px = -dy;
	float py = dx;

	if(inverted){
		px = -px;
		py = -py;
	}
  
	// Normaliza o perpendicular
	float length = sqrtf(px * px + py * py);
	px /= length;
	py /= length;
  
	// Escala o perpendicular pelo comprimento do lado
	float sideLength = sqrtf(dx * dx + dy * dy);
	px *= sideLength;
	py *= sideLength;
  
	// Cores azuis
	glColor3f(0.0f, 1.0f, 0.0f);
  
	// Desenha o quadrado
	glBegin(GL_QUADS);
	    glVertex3f(x1, y1, 0);                 // Base esquerda
	    glVertex3f(x2, y2, 0);                 // Base direita
	    glVertex3f(x2 + px, y2 + py, 0);       // Topo direito deslocamos x2(um ponto da base do triangulo) na direcao do vetor perpendicular
	    glVertex3f(x1 + px, y1 + py, 0);       // Topo esquerdo
	glEnd();
}
void CalculateUpperDotTriangle(float ax, float ay, float bx, float by){

	// // Calcula ponto do meio da base
	float middleX = (ax + bx)/ 2.0f;
	float middleY = (ay + by) / 2.0f;

	//vetor perpendicular a base
	float perpX = -(by -ay);
	float perpY = (bx - ax);

	// Normaliza
	float length = sqrt(perpX * perpX + perpY * perpY);
	perpX /= length;
	perpY /= length;
	
	float height = sqrt(3)/2.0; // altura de um triângulo equilátero com lado 

	// Coordenadas do ponto superior (cx, cy)
	float cx = middleX + perpX * height;
	float cy = middleY + perpY * height;

	glColor3f(0.0f, 1.0f, 0.0f);
	DesenhaQuadradoNaAresta(ax, ay,bx, by, 1); //todos os 3 precisam estar invertidos pq o vetor perpendicular tem q apontar pra fora
	DesenhaQuadradoNaAresta(bx, by, cx, cy, 1);
	DesenhaQuadradoNaAresta(cx, cy, ax, ay, 1);

	glColor3f(0.0f, 0.0f, 1.0f);
	glBegin(GL_TRIANGLES);
		glVertex3f(ax, ay, 0);
		glVertex3f(bx, by, 0);
		glVertex3f(cx, cy, 0);
	glEnd();

	
    
}



void DesenhaTriangulo(){
    
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
	gluOrtho2D(-1.5, 1.5, -1.5, 1.5);
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
 
	return 0;
}
