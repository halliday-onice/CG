// TeaPot3D.cpp - Isabel H. Manssour (adaptado para C++ por ChatGPT)
// Programa OpenGL que exemplifica a visualização de objetos 3D em C++ no macOS
#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>  // Use GLUT/glut.h no macOS
#include<iostream>
#include<OpenGL/gl.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

GLfloat angle = 45.0f;
GLfloat fAspect;

class Camera {
      private:
            glm::mat4 viewMatrix;
            glm::vec3 position;
            glm::vec3 front;
            glm::vec3 right;
            glm::vec3 up;
            glm::vec3 cameraTarget;

            GLfloat pitch;
            GLfloat yaw;
            GLfloat roll;

            

      public:
            Camera(glm::vec3 pos, glm::vec3 target){
                  position = pos;      
                  front = glm::normalize(target - position); // Subtracting the camera position vector from the scene's origin vector thus results in the direction vector we want
                  up = glm::vec3(0.0f, 1.0f, 0.0f);
            }
            const glm::mat4 getViewMatrix() {
                  //lookAt(cameraPosition, targetPosition, upDirection)
                  this->viewMatrix = glm::lookAt(position, position + front, up);
                  return this->viewMatrix;
            }


            void printCameraPosition() const {
                  std::cout << "Camera position: ("<<position.x << "," <<position.y <<"," <<position.z << ")\n";
            }
            const glm::vec3 getPosition() const {
                  return this-> position;
            }

            
};

// ------------------------------------- Instantiating the cameras ------------------------------------
Camera camera1(
  glm::vec3(0.0f,80.0f, 200.0f), 
  glm::vec3(-60.0f, 0.0f, 0.0f)// if I want the camera to look at the teapot, it makes sense to use that same position 
);

Camera camera2(
  glm::vec3(0.0f, 90.0f, 250.0f), //different position for the camera
  glm::vec3(105.0, 10.0, 0.0)//position of the sphere
);

Camera* activeCamera = &camera1;
// ------------------------------------- Instantiating the cameras ------------------------------------
// Função callback chamada para fazer o desenho
void Draw()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glLoadIdentity();
    // carregar a matriz de visualização calculada pela câmera (camera.getViewMatrix())
    glm::mat4 view = activeCamera->getViewMatrix();
    glLoadMatrixf(&view[0][0]);
    
    //desenha o bule de chá
    glPushMatrix();
      glColor3f(0.0f, 0.0f, 1.0f);
      glTranslatef(-60.0, 0.0, 0.0); // Translada o teapot para a origem
      glColor3f(0.0f, 0.0f, 1.0f);
      glutWireTeapot(50.0);  // Desenha o teapot em wireframe
    glPopMatrix();

      //desenha o cubo
      glPushMatrix();
        glColor3f(1.0f, 0.0f, 0.0f);
        glTranslatef(60.0, 0.0, 0.0); // Translada o cubo para a origem
        glutWireCube(50.0); // Desenha o cubo em wireframe
      glPopMatrix();

      //desenha a esfera
      glPushMatrix();
        glColor3f(1.0f, 1.0f, 0.0f);
        glTranslatef(105.0, 10.0, 0.0); // Translada a esfera para a origem
        glutWireSphere(20.0, 50.0, 20); // Desenha a esfera em wireframe
      glPopMatrix();

    glutSwapBuffers();     // Executa os comandos OpenGL
}

// Inicializa parâmetros de rendering
void Inicializa()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

// Função usada para especificar o volume de visualização
void EspecificaParametrosVisualizacao()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(angle, fAspect, 0.5, 500.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}

// Função callback chamada quando o tamanho da janela é alterado
void AlteraTamanhoJanela(GLsizei w, GLsizei h)
{
    if (h == 0) h = 1;

    glViewport(0, 0, w, h);

    fAspect = static_cast<GLfloat>(w) / static_cast<GLfloat>(h);
    EspecificaParametrosVisualizacao();
}

// Função callback chamada para gerenciar eventos do mouse
void GerenciaMouse(int button, int state, int x, int y)
{

  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        std::cout << "[Mouse] Left Click - Keeping current camera\n";
  }
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
      if(activeCamera == &camera1){
        activeCamera = &camera2;
        std::cout << "[Mouse] Switched to Camera 2 (Sphere)\n";
      } else {
        activeCamera = &camera1;
        std::cout << "[Mouse] Switched to Camera 1 (Teapot)\n";
      }
      activeCamera->printCameraPosition();
      glutPostRedisplay(); //force OpenGL to redraw the scene with the new camera orientation 
  }
}
    

// Programa principal
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(550, 300);
    glutCreateWindow("Visualizacao 3D");

    glutDisplayFunc(Draw);
    glutReshapeFunc(AlteraTamanhoJanela);
    glutMouseFunc(GerenciaMouse);

    Inicializa();
    glutMainLoop();

    return 0;
}
