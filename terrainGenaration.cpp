#define GL_SILENCE_DEPRECATION
#include <iostream>
#include <cmath>
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <vector>
#include <unordered_map>
#include <memory> // <- necessário para unique_ptr

const int w = 1400;
const int h = 1000;
const int scl = 20;
const int columns = w / scl;
const int rows = h / scl;
float terrain[columns][rows];

struct Vertex {
    float x, y, z;

    bool operator==(const Vertex& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

struct Triangle {
    Vertex v1, v2, v3;
    Triangle(Vertex a, Vertex b, Vertex c) : v1(a), v2(b), v3(c) {}
};

std::vector<Triangle> triangles;

struct Edge {
    int v1Index, v2Index;
    Edge(int a, int b) {
        v1Index = std::min(a, b);
        v2Index = std::max(a, b);
    }
};

struct EdgeNode {
    Edge edge;
    std::unique_ptr<EdgeNode> next;
    EdgeNode(Edge e) : edge(e), next(nullptr) {}
};

struct VertexEdges {
    std::unique_ptr<EdgeNode> head;

    VertexEdges() = default;

    void addEdge(const Edge& e) {
        std::unique_ptr<EdgeNode> newNode = std::make_unique<EdgeNode>(e);
        newNode->next = std::move(head);
        head = std::move(newNode);
    }
};

std::vector<Vertex> vertexList;
std::unordered_map<std::string, int> vertexIndexMap;
std::vector<VertexEdges> vertexEdgeList;

struct Camera {
    float camX = 700.0f, camY = 800.0f, camZ = 1200.0f;
    float targetX = 700.0f, targetY = 0.0f, targetZ = 550.0f;

    void moveForward(float passo) {
        float dx = targetX - camX;
        float dy = targetY - camY;
        float dz = targetZ - camZ;

        float len = sqrt(dx*dx + dy*dy + dz*dz);
        if (len > 0.001f) {
            dx /= len; dy /= len; dz /= len;
            camX += dx * passo;
            camY += dy * passo;
            camZ += dz * passo;
            targetX += dx * passo;
            targetY += dy * passo;
            targetZ += dz * passo;
        }
    }
    void rotateY(float angle) {
        float rad = angle * M_PI / 180.0f;
        float cosA = cos(rad);
        float sinA = sin(rad);

        float dx = targetX - camX;
        float dz = targetZ - camZ;

        targetX = camX + dx * cosA - dz * sinA;
        targetZ = camZ + dx * sinA + dz * cosA;
    }

    void applyView() {
        gluLookAt(camX, camY, camZ, targetX, targetY, targetZ, 0.0f, 1.0f, 0.0f);
    }
};

Camera camera;

void initTerrain() {
    for (int x = 0; x < columns; x++) {
        for (int y = 0; y < rows; y++) {
           terrain[x][y] = 30 * sin(x * 0.1f) * cos(y * 0.1f) + 15 * sin(x * 0.5f) * sin(y * 0.3f) + 8  * cos(x * 1.5f + y * 0.2f);

        }
    }
}

std::string makeKey(const Vertex& v) {
    return std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z);
}

int getOrAddVertexIndex(const Vertex& v) {
    std::string key = makeKey(v);
    if (vertexIndexMap.find(key) == vertexIndexMap.end()) {
        int index = vertexList.size();
        vertexList.push_back(v);
        vertexEdgeList.emplace_back();
        vertexIndexMap[key] = index;
    }
    return vertexIndexMap[key];
}

void printEdgesForVertex(int vertexIndex) {
    if (vertexIndex < 0 || vertexIndex >= vertexEdgeList.size()) {
        std::cout << "Índice de vértice inválido.\n";
        return;
    }

    const Vertex& v = vertexList[vertexIndex];
    std::cout << "Vértice [" << vertexIndex << "] (" << v.x << ", " << v.y << ", " << v.z << ") está conectado a:\n";

    EdgeNode* node = vertexEdgeList[vertexIndex].head.get(); // usando unique_ptr agora
    while (node) {
        std::cout << "  Aresta entre vértices [" << node->edge.v1Index << "] e [" << node->edge.v2Index << "]\n";
        node = node->next.get();
    }
}


void generateTerrain() {
    for (int y = 0; y < rows - 1; y++) {
        for (int x = 0; x < columns - 1; x++) {
            Vertex v0 = { static_cast<float>(x * scl), terrain[x][y], static_cast<float>(y * scl) };
            Vertex v1 = { static_cast<float>(x * scl),terrain[x][y+1], static_cast<float>((y+1) * scl) };
            Vertex v2 = { static_cast<float>((x+1) * scl),terrain[x+1][y], static_cast<float>(y * scl) };
            Vertex v3 = { static_cast<float>((x+1) * scl),terrain[x+1][y+1],static_cast<float>((y+1) * scl) };

            int i0 = getOrAddVertexIndex(v0);
            int i1 = getOrAddVertexIndex(v1);
            int i2 = getOrAddVertexIndex(v2);
            int i3 = getOrAddVertexIndex(v3);

            triangles.push_back(Triangle(v0, v1, v2));
            triangles.push_back(Triangle(v2, v1, v3));

            vertexEdgeList[i0].addEdge(Edge(i0, i1));
            vertexEdgeList[i1].addEdge(Edge(i1, i2));
            vertexEdgeList[i2].addEdge(Edge(i2, i0));
            vertexEdgeList[i2].addEdge(Edge(i2, i1));
            vertexEdgeList[i1].addEdge(Edge(i1, i3));
            vertexEdgeList[i3].addEdge(Edge(i3, i2));
        }
    }
}

void drawTerrain() {
    glColor3f(0.2f, 0.6f, 0.3f);
    glPushMatrix();
    glTranslatef(-w / 2, -h / 2, 0);
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
        case 27: exit(0); break;
        case 'w': camera.moveForward(10); break;
        case 's': camera.moveForward(-10); break;
        case 'a': camera.rotateY(-5); break;  //left
        case 'd': camera.rotateY(5);  break;  //right
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
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f);

    initTerrain();
    generateTerrain();

    // ✅ Teste da lista de arestas do vértice 0
    printEdgesForVertex(0);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(handleKeyboard);

    glutMainLoop();
    return 0;
}
