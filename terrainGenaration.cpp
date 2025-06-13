#define GL_SILENCE_DEPRECATION
#include <iostream>
#include <cmath>
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <vector>

const int w = 1400;
const int h = 1000;
const int scl = 20; // the "size" of each line, it is the scale of it
const int columns = w / scl;
const int rows = h / scl;
float terrain[columns][rows];

struct Vertex {
    float x, y, z;
};
struct Triangle {
    Vertex v1, v2, v3;

    Triangle(Vertex vertex1, Vertex vertex2, Vertex vertex3) 
        : v1(vertex1), v2(vertex2), v3(vertex3) {}
    
    
};
/* I need to store each of vertex of the triangles and 
to each edges the vertex is connected too. This is to reproduce the effect 
of flowing until the point that is the lowest possible */
struct triangleNode {
    Triangle data;
    triangleNode* next;
    triangleNode(Triangle t) : data(t), next(nullptr) {}
};
// Linked list to store triangles
class TriangleList {
public:
    triangleNode* head;

    TriangleList() : head(nullptr) {}

    void addTriangle(const Triangle& t) {
        triangleNode* newNode = new triangleNode(t);
        if (!head) {
            head = newNode;
        } else {
            triangleNode* current = head;
            while (current->next) {
                current = current->next;
            }
            current->next = newNode;
        }
    }


    void clear() {
        triangleNode* current = head;
        while (current) {
            triangleNode* toDelete = current;
            current = current->next;
            delete toDelete;
        }
        head = nullptr;
    }
    ~TriangleList() {
        clear();
    }
};


struct Edge {
    int v1Index, v2Index;

    Edge(int index1, int index2) : v1Index(index1), v2Index(index2) {}
    
};
struct edgeNode {
    Edge data;
    edgeNode* next;

    edgeNode(Edge e) : data(e), next(nullptr) {}
};

struct vertexEdge {
    edgeNode* head;
    vertexEdge() : head(nullptr) {}
    void addEdge(const Edge& e) {
        edgeNode* newNode = new edgeNode(e);
        if (!head) {
            head = newNode;
        } else {
            edgeNode* current = head;
            while (current->next) {
                current = current->next;
            }
            current->next = newNode;
        }
    }
    void clear() {
        edgeNode* current = head;
        while (current) {
            edgeNode* toDelete = current;
            current = current->next;
            delete toDelete;
        }
        head = nullptr;
    }
    ~vertexEdge() {
        clear();
    }
};


std::vector<Triangle> triangles;
std::vector<vertexEdge> vertexEdgeList;
std::vector<Vertex> vertices;
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

void generateTerrain() {
    // Generate terrain height data
    for (int y = 0; y < rows - 1; y++) {
        std:: vector<Vertex> currentVertices;
        for (int x = 0; x < columns; x++) {
            //adiciona o vertice (x, y)
            currentVertices.push_back(Vertex{static_cast<float>(x * scl), static_cast<float>(terrain[x][y]), static_cast<float>(y * scl)});
          
          //adiciona o vertice (x, y+1)
            currentVertices.push_back(Vertex{static_cast<float>(x * scl), static_cast<float>(terrain[x][y + 1]), static_cast<float>((y + 1) * scl)});

          //a partir do terceiro vertice, adiciona o triangulo
          if(currentVertices.size() >= 3) {
            // Determinando os três vértices do triângulo atual
                Vertex v1 = currentVertices[currentVertices.size() - 3];
                Vertex v2 = currentVertices[currentVertices.size() - 2];
                Vertex v3 = currentVertices[currentVertices.size() - 1];
                triangles.push_back(Triangle(v1, v2, v3));

                //OpenGL alterna a ordem dos vértices para formar triângulos
                // triangulo com indice par dentro do strips sao (0, 2, 4 ...)
                // triangulo com indice impar dentro do strips sao (1, 3, 5 ...)
                // Isso é necessário para garantir que os triângulos sejam desenhados corretamente
                if (triangles.size() % 2 == 0) {
                    triangles.push_back(Triangle(v1, v3, v2));
                } else {
                    triangles.push_back(Triangle(v2, v1, v3));
                }
                std::cout << "Triangle: \n";
                std::cout << "  v1: (" << v1.x << ", " << v1.y << ", " << v1.z << ")\n";
                std::cout << "  v2: (" << v2.x << ", " << v2.y << ", " << v2.z << ")\n";
                std::cout << "  v3: (" << v3.x << ", " << v3.y << ", " << v3.z << ")\n";
                std::cout << "---------------------------\n";
            }
            


          
        }
    }
}

void drawTerrain() {
    // Conventional coordinate system: XZ plane, Y up
    glColor3f(0.2f, 0.6f, 0.3f);  // Set terrain color (green)
    //aqui vamos armazenar os vértices do triangulo
    glPushMatrix();
    glTranslatef(-w / 2, -h / 2, 0); // Center the terrain

    for (const Triangle& t : triangles) {
        glBegin(GL_TRIANGLES);
        glVertex3f(t.v1.x, t.v1.y, t.v1.z);
        glVertex3f(t.v2.x, t.v2.y, t.v2.z);
        glVertex3f(t.v3.x, t.v3.y, t.v3.z);
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
    
    initTerrain(); 
    generateTerrain(); // Generate terrain data, precisa ser chamado após initTerrain
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(handleKeyboard);
    
    glutMainLoop();
    return 0;
}