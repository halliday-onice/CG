#define GL_SILENCE_DEPRECATION
#include <iostream>
#include <iomanip>
#include <cmath>
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <vector>
#include <unordered_map>
#include <memory> // <- necessário para unique_ptr



const int WIDTH = 1400;
const int HEIGHT = 1000;
const int TERRAIN_SCALE = 20;
const int COLUMNS = WIDTH/ TERRAIN_SCALE;
const int ROWS = HEIGHT / TERRAIN_SCALE;


struct Vertex {
    float x, y, z;
};

struct Particle {
    float x,y,z;
    int currentVertexIndex = -1; // inicializa com -1 porque ainda não conhece o proximo destino válido para se mover
};


Particle particle;
std::vector<Vertex> vertices;
std::vector<int> indices;
std::vector<std::vector<int>> adjencyList; //cria uma lista de vertices, e pra cada um deles, tem uma lista dos vizinhos
//o indice da lista externa std::vector< ... > eh o indice do vertice que estamos consultando atualmente
//a lista interna std::vector<int>
// O índice da lista externa corresponde ao índice do vértice que estamos consultando


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

std::string makeKey(const Vertex& v) {
    return std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z);
}


void generateTerrain() {
    //primeiro gerar todos os vertices da grade
    vertices.reserve(COLUMNS * ROWS);
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLUMNS; x++) {
            float height = -300 * sin(x * 0.1f) * cos(y * 0.1f) + 150 * sin(x * 0.5f) * sin(y * 0.3f) + 80 * cos(x * 1.5f + y * 0.2f);
            Vertex v;
            v.x = x * TERRAIN_SCALE;
            v.y = height;
            v.z = y * TERRAIN_SCALE;
            vertices.push_back(v);
        }
    }
    adjencyList.assign(vertices.size(), std::vector<int>()); // Apague todo o conteúdo de adjacencyList. Depois, preencha adjacencyList com vertices.size()  cópias de um novo vetor de inteiros vazio.
    //O vetor indices armazena os "endereços" dos vértices que formam cada triângulo
    // cada um triangulo precisa de 3 indices para os vertices, e cada quadrado tem 2 triangulos
    //logo 2 * 3 indices por quadrado
    indices.reserve((ROWS - 1) * (COLUMNS - 1) * 6);

    //gerar os indices dos vertices e a lista de adj
    for(int y = 0; y < ROWS - 1; y++){
        for(int x = 0; x <COLUMNS - 1; x++){
            //pegar os 4 indices do quadrado
            int topLeft = y * COLUMNS + x; //top left
            int bottomLeft = (y + 1) * COLUMNS + x;
            int topRight =  y * COLUMNS + (x + 1);
            int bottomRight = (y + 1) * COLUMNS + (x + 1);

            //adiciono os dois triangulos para renderezicao
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);

            // adiciona as conexoes
            //
            adjencyList[topLeft].push_back(bottomLeft); adjencyList[bottomLeft].push_back(topLeft);
            adjencyList[topLeft].push_back(topRight); adjencyList[topRight].push_back(topLeft);
            adjencyList[bottomLeft].push_back(topRight); adjencyList[topRight].push_back(bottomLeft); // Aresta diagonal
            adjencyList[bottomLeft].push_back(bottomRight); adjencyList[bottomRight].push_back(bottomLeft);
            adjencyList[topRight].push_back(bottomRight); adjencyList[bottomRight].push_back(topRight);
        }
    }
}

void drawTerrain() {
    glPushMatrix();
    glTranslatef(-WIDTH/ 2.0f, -HEIGHT/2.0f, 0.0f);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), vertices.data());
    glColor3f(0.2f, 0.6f, 0.3f);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, indices.data());
    glDisableClientState(GL_VERTEX_ARRAY);

    glPopMatrix();
}

void drawParticle(){
    glPushMatrix();
    glTranslatef(-WIDTH / 2.0f, -HEIGHT / 2.0f, 0);
    glTranslatef(particle.x, particle.y, particle.z);
    glColor3f(1.0f, 0.0f, 0.0f);
    glutSolidSphere(10.0, 16, 16);
    glPopMatrix();
}


void movementInfo(int fromIndex, int toIndex){
    //check for valid indices
    if(fromIndex < 0 || fromIndex >= vertices.size() || toIndex < 0 || toIndex >= vertices.size()){
        return;
    }


    const Vertex& fromVertex = vertices[fromIndex];
    const Vertex& toVertex = vertices[toIndex];


    // Configura o std::cout para imprimir números de ponto flutuante com 2 casas decimais
    std::cout << std::fixed << std::setprecision(2);
    
    std::cout << "Movendo particula:" << std::endl;
    std::cout << "  De (Atual):  Vertice [" << fromIndex << "] | Altura: " << fromVertex.y << std::endl;
    std::cout << "  Para (Proximo): Vertice [" << toIndex << "] | Altura: " << toVertex.y << std::endl;
    std::cout << "------------------------------------------" << std::endl;
}

void resetParticle() {
    if (vertices.empty()) 
        return;
    particle.currentVertexIndex = rand() % vertices.size();
    const auto& startVertex = vertices[particle.currentVertexIndex];
    particle.x = startVertex.x;
    particle.y = startVertex.y;
    particle.z = startVertex.z;
}

void handleKeyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 27: exit(0); break;
        case 'w': camera.moveForward(10); break;
        case 's': camera.moveForward(-10); break;
        case 'a': camera.rotateY(-5); break;  //left
        case 'd': camera.rotateY(5);  break;  //right
        case 'r': resetParticle(); break;
    }
    glutPostRedisplay();
}
void flowSimulation(int value){
    if (particle.currentVertexIndex < 0){
        resetParticle();
    }

    int currentIndex = particle.currentVertexIndex;
    const auto& neighbors = adjencyList[currentIndex];

    int nextIndexToMove = -1;

    float minimumHeight = vertices[currentIndex].y;

    //procurando pela menor altura
    for(int indexOfNeighbor: neighbors){
        if(vertices[indexOfNeighbor].y < minimumHeight ){ //busca por um vizinho mais baixo, se achar atualizo o indice para p vertice 
            minimumHeight  = vertices[indexOfNeighbor].y;
            nextIndexToMove = indexOfNeighbor;
        }
    }
    //se nextIndexToMove for igual a -1, nao foi encontrado um vizinho mais baixo que a posicao atual
    if (nextIndexToMove != -1){
        movementInfo(currentIndex, nextIndexToMove);
        particle.currentVertexIndex = nextIndexToMove; // "faça com que a animacao va para a direcao do"
        const auto& nextVertex = vertices[nextIndexToMove]; // forma moderna e eficiente em C++ de se referir a um objeto sem criar uma cópia dele, o que torna o código mais rápido
        //atualiza as posicoes, visualmente na animacao
        particle.x = nextVertex.x;
        particle.y = nextVertex.y;
        particle.z = nextVertex.z;
    }

    glutPostRedisplay(); // é o comando que garante que a função display() seja chamada para desenhar a partícula em sua nova posição
    glutTimerFunc(100, flowSimulation, 0);
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
    drawParticle();
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

    generateTerrain();
    resetParticle();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(handleKeyboard);
    glutTimerFunc(100, flowSimulation, 0);

    glutMainLoop();
    return 0;
}
