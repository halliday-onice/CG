#define GL_SILENCE_DEPRECATION
#include <iostream>
#include <cmath>
#include <GLUT/glut.h>
#include <OpenGL/gl.h>

const int w = 1400;
const int h = 1000;
const int scl = 20; // the "size" of each line, it is the scale of it
const int columns = w / scl;
const int rows = h / scl;
float terrain[columns][rows];

struct Vertex {
    float v1, v2, v3;
};

class Camera {
public:
    float camX = 0.0f, camY = 300.0f, camZ = 300.0f;
    float targetX = 0.0f, targetY = 0.0f, targetZ = 0.0f;

    void moveForward(float passo) {
        float dx = targetX - camX;
        float dy = targetY - camY;
        float dz = targetZ - camZ;

        float len = sqrt(dx*dx + dy*dy + dz*dz);
        if (len > 0.001f) {
            dx = dx / len;
            dy = dy / len;
            dz = dz / len;

            camX += dx * passo;
            camY += dy * passo;
            camZ += dz * passo;

            targetX += dx * passo;
            targetY += dy * passo;
            targetZ += dz * passo;
        }
    }

    void applyView() {
        gluLookAt(camX, camY, camZ,
                  targetX, targetY, targetZ,
                  0.0f, 1.0f, 0.0f);
    }
};

Camera camera;

void initTerrain() {
    // Initialize with simple height pattern
    for (int x = 0; x < columns; x++) {
        for (int y = 0; y < rows; y++) {
            terrain[x][y] = 10 * sin(x*0.2f) * cos(y*0.2f);
        }
    }
}

void drawTerrain() {
    glColor3f(0.2f, 0.6f, 0.3f);  // Set terrain color (green)
    glPushMatrix();
    glTranslatef(-w/2, 0, -h/2);  // Center terrain at origin
    
    for (int y = 0; y < rows - 1; y++) {
        glBegin(GL_TRIANGLE_STRIP);
        for (int x = 0; x < columns; x++) {
            // Conventional coordinate system: XZ plane, Y up
            glVertex3f(x * scl, terrain[x][y], y * scl);
            glVertex3f(x * scl, terrain[x][y+1], (y+1) * scl);
        }
        glEnd();
    }
    glPopMatrix();
}

void handleKeyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 27: exit(0); break;          // ESC
        case 'w': camera.moveForward(10); break;
        case 's': camera.moveForward(-10); break;
    }
    glutPostRedisplay();
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)width/height, 1.0, 3000.0);
    glMatrixMode(GL_MODELVIEW);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    camera.applyView();
    drawTerrain();

    glutSwapBuffers();
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Terrain Generation");
    
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Render polygons as wireframe
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f);  // Sky blue
    
    //initTerrain();  // Initialize height data
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(handleKeyboard);
    
    glutMainLoop();
    return 0;
}