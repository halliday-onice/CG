#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>  // Use GLUT/glut.h no macOS
#include<iostream>
#include <cmath>  // Para C++
#include<OpenGL/gl.h>

// Terrain parameters
const float WIDTH = 10.0f;   // X-axis size
const float DEPTH = 10.0f;   // Z-axis size
const int GRID_SIZE = 20;    // Number of segments




bool perspectiveMode = true; //starts with perspective
class Camera{
    public:
        //camera position
        float camX = 8.0f, camY = 5.0f, camZ = 8.0f;
        float targetX = 0.0f, targetY = 0.0f, targetZ = 0.0f;

        Camera()

        : camX(8.0f), camY(5.0f), camZ(8.0f),
          targetX(0.0f), targetY(0.0f), targetZ(0.0f) {}
        
        void moveForward(float passo){
            //componentes da direcao que a camera esta olhando
            float dx = targetX - camX;
            float dy = targetY - camY;
            float dz = targetZ - camZ;

            //normalize o vetor
            float len = sqrt(dx * dx + dy * dy + dz * dz);
            dx = dx/len;
            dy = dy/len;
            dz = dz/len;

            //atualizando a posicao da camera
            camX += dx * passo;
            camY += dy * passo;
            camZ += dz * passo;

            //novo alvo
            targetX += dx * passo; //poderia escrever targetX = targetX + dx * passo pra ficar mais parecido com a tarefa
            targetY += dy * passo;
            targetZ += dz * passo;


        }

        void applyView(){
            gluLookAt(camX, camY, camZ,
            targetX, targetY, targetZ,
            0.0f, 1.0f, 0.0f);
        }


        // essa funcao ira fazer olhar para os lados
        void rotateY(float angle){
            float angleRadians = angle * M_PI/ 180.0f;

            //quando rotaciona em torno do eixo x, a gnt tem rotação no eixo YZ
            float dx = targetX - camX;
            float dz = targetZ - camZ;

            //aplica a rotacao, nesse caso nao ira rotacionar para cima

            float newDegreeX = dx * cos(angleRadians) + dz * sin(angleRadians);
            float newDegreeZ = - dz * sin(angleRadians) + dz * cos(angleRadians);

            targetX = camX + newDegreeX;
            targetZ = camZ + newDegreeZ;


        }
    

}; 

void init() {
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f); // Sky blue background
    glEnable(GL_DEPTH_TEST);
    
    // Basic lighting setup
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat lightPos[4] = {5, 10, 5, 1.0};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    
    // Material properties
    GLfloat green[] = {0.0f, 0.6f, 0.0f, 1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green);
}

void drawTerrain() {
    float halfWidth = WIDTH / 2.0f;
    float halfDepth = DEPTH / 2.0f;
    float stepX = WIDTH / GRID_SIZE;
    float stepZ = DEPTH / GRID_SIZE;

    // Draw the terrain mesh
    for (int z = 0; z < GRID_SIZE; z++) {
        glBegin(GL_QUADS);
        for (int x = 0; x < GRID_SIZE; x++) {
            // Calculate vertex positions
            float x1 = -halfWidth + x * stepX;
            float x2 = x1 + stepX;
            float z1 = -halfDepth + z * stepZ;
            float z2 = z1 + stepZ;
            
            // Define quad vertices (counter-clockwise order)
            glNormal3f(0, 1, 0); // Flat terrain normals point upward
            
            glVertex3f(x1, 0.0f, z1); // Bottom-left
            glVertex3f(x2, 0.0f, z1); // Bottom-right
            glVertex3f(x2, 0.0f, z2); // Top-right
            glVertex3f(x1, 0.0f, z2); // Top-left
        }
        glEnd();
    }
}
Camera camera;

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    camera.applyView();
    
    drawTerrain();

    glPushMatrix();
        glEnable(GL_LIGHTING);
        glTranslatef(-2.0f, 0.5f, -1.0f);
        glColor3f(1.0f, 0.0f, 0.0f);
        glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
        glEnable(GL_LIGHTING);
        glTranslatef(2.0f, 1.8f, 2.0f);
        glColor3f(0.8f, 0.5f, 0.2f);
        glutSolidTeapot(0.6); //radius 0.6
    glPopMatrix();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float aspect = (float) w / h;
    if(perspectiveMode){
        gluPerspective(45.0, (float)w / h, 0.1, 100.0);
    } else {
        float size = 10.0f;
        glOrtho(-size * aspect, size * aspect, -size, size, 0.1, 100); // glOrtho(left, right, bottom, top, near, far);  Essa "caixa" define o espaço visível na tela, como se fosse uma câmera 2D "chapada".
    }
   
    glMatrixMode(GL_MODELVIEW);
}

void handleKeyboard(unsigned char key, int x, int y){
    switch(key){
        case 27: //esc key
            exit(0);
            break;
        case 'p': // Alterna modo de projeção
        case 'P':{
            perspectiveMode = !perspectiveMode;
            int width = glutGet(GLUT_WINDOW_WIDTH);
            int height = glutGet(GLUT_WINDOW_HEIGHT);
            reshape(width, height);  // Apply the new projection
            glutPostRedisplay();     // Redraw the scene It tells OpenGL/GLUT that the window needs to be redrawn as soon as possible.
            break;
        }
        case 'w':{ //pra frente
            camera.moveForward(0.5f);
            glutPostRedisplay();
            break;
        }

        case 'd': { //rotaciona para esquerd
            camera.rotateY(-5.0f);
            glutPostRedisplay();
            break;
        }
        case 'a': {
            camera.rotateY(5.0f);
            glutPostRedisplay();
            break;
        }   
        

    }
}


int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("OpenGL Legacy Terrain");
    
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(handleKeyboard);
    glutMainLoop();
    return 0;
}