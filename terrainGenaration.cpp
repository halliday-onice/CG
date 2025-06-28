#define GL_SILENCE_DEPRECATION
#include <iostream>
#include <iomanip>
#include <cmath>
//#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <vector>
#include <unordered_map>
#include <memory> // <- necessário para unique_ptr
#include "mybib.h"
#include <algorithm>

// para compilar importando o terreno: 
//  g++ terrainGenaration.cpp mybib.c -o terrain -framework OpenGL -framework GLUT 
//  vai dar um warning por conta da diferenca de c e c++


const int WIDTH = 1400;
const int HEIGHT = 1000;
const int TERRAIN_SCALE = 20; //isso aqui eh pra o terreno nao ter 
const int COLUMNS = WIDTH/ TERRAIN_SCALE;
const int ROWS = HEIGHT / TERRAIN_SCALE;

struct Particle {
    float x,y,z;
    int currentVertexIndex = -1; // inicializa com -1 porque ainda não conhece o proximo destino válido para se mover
};

Camera camera;
Particle particle;
std::vector<Vertex> vertices;
std::vector<int> indices;
std::vector<std::vector<int>> adjencyList; //cria uma lista de vertices, e pra cada um deles, tem uma lista dos vizinhos
//o indice da lista externa std::vector< ... > eh o indice do vertice que estamos consultando atualmente
//a lista interna std::vector<int>
// O índice da lista externa corresponde ao índice do vértice que estamos consultando
ObjModel model;


void cameraApplyView(const Camera* cam) {
    gluLookAt(cam->px, cam->py, cam->pz, cam->tx, cam->ty, cam->tz, 0.0f, 1.0f, 0.0f);
}

void cameraMoveForward(Camera* cam, float passo) {
    float dx = cam->tx - cam->px;
    float dz = cam->tz - cam->pz;
    float len = sqrt(dx*dx + dz*dz); // Movimento apenas no plano XZ
    if (len > 0.001f) {
        dx /= len;
        dz /= len;
        cam->px += dx * passo;
        cam->pz += dz * passo;
        cam->tx += dx * passo;
        cam->tz += dz * passo;
    }
}

void cameraRotateY(Camera* cam, float angle) {
    float rad = angle * 3.14159f / 180.0f;
    float cosA = cos(rad);
    float sinA = sin(rad);

    float dx = cam->tx - cam->px;
    float dz = cam->tz - cam->pz;

    cam->tx = cam->px + dx * cosA - dz * sinA;
    cam->tz = cam->pz + dx * sinA + dz * cosA;
}

void resetParticle() {
    //desativo a particula, colocando o currentVertexIndex = -1
    particle.currentVertexIndex = -1;
    std::cout << "Particle was reseted" << std::endl;
}


// =======================
// Geração de um terreno pelo código
// =======================
void generateProceduralTerrain() {
    vertices.clear();
    indices.clear();
    adjencyList.clear();

    // Gera os vértices da grade
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLUMNS; x++) {
            float height = 30 * sin(x * 0.1f) * cos(y * 0.1f) + 15 * sin(x * 0.5f) * sin(y * 0.3f) + 8 * cos(x * 1.5f + y * 0.2f); // obtido usando testes
            vertices.push_back({(float)(x * TERRAIN_SCALE), height, (float)(y * TERRAIN_SCALE)});
        }
    }

    // Prepara a lista de adjacência
    adjencyList.assign(vertices.size(), std::vector<int>());

    // Gera os índices e a lista de adjacência
    for (int y = 0; y < ROWS - 1; y++) {
        for (int x = 0; x < COLUMNS - 1; x++) {
            int topLeft = y * COLUMNS + x;
            int bottomLeft = (y + 1) * COLUMNS + x;
            int topRight = y * COLUMNS + (x + 1);
            int bottomRight = (y + 1) * COLUMNS + (x + 1);

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);

            adjencyList[topLeft].push_back(bottomLeft); adjencyList[bottomLeft].push_back(topLeft);
            adjencyList[topLeft].push_back(topRight); adjencyList[topRight].push_back(topLeft);
            adjencyList[bottomLeft].push_back(topRight); adjencyList[topRight].push_back(bottomLeft);
            adjencyList[bottomLeft].push_back(bottomRight); adjencyList[bottomRight].push_back(bottomLeft);
            adjencyList[topRight].push_back(bottomRight); adjencyList[bottomRight].push_back(topRight);
        }
    }
    
    // Translada o terreno para a origem para que a câmera padrão funcione bem
    for (auto& v : vertices) {
        v.x -= WIDTH / 2.0f;
        v.z -= HEIGHT / 2.0f;
    }
}

// ===================================================================
// MÉTODO 2: Importa modelo .obj
// ===================================================================
void processImportedModel(ObjModel *model) {
    vertices.clear();
    indices.clear();
    adjencyList.clear();

    if (!model || model->vertexCount == 0) 
        return;

    // (mybib.c) box É a menor caixa retangular que contém o objeto inteiro.
    // Lógica para centralizar e escalonar o modelo
    float centerX = (model->box.minX + model->box.maxX) / 2.0f;
    float centerY = (model->box.minY + model->box.maxY) / 2.0f;
    float centerZ = (model->box.minZ + model->box.maxZ) / 2.0f;
    float sizeX = model->box.maxX - model->box.minX; //subtrai a coordenada minima da máxima
    float sizeY = model->box.maxY - model->box.minY;
    float sizeZ = model->box.maxZ - model->box.minZ;
    float modelSize = std::max({sizeX, sizeY, sizeZ}); //fazemos isso pra escalar de forma uniforme
    float scaleFactor = (modelSize > 0) ? 100.0f / modelSize : 1.0f; // a maior dimensao do objeto importado tenha o tamanho padrao de 100 fator = tamanho_desejado / tamanho_atual

    // Processa os vértices com a transformação
    vertices.reserve(model->vertexCount);


    for (int i = 0; i < model->vertexCount; i++) {
        float newX = (model->vertices[i].x - centerX) * scaleFactor;
        float newY = (model->vertices[i].y - centerY) * scaleFactor;
        float newZ = (model->vertices[i].z - centerZ) * scaleFactor;
        vertices.push_back({newX, newY, newZ});
    }

    // Preenche a lista de adjacência e os índices a partir das faces
    adjencyList.assign(vertices.size(), std::vector<int>());
    indices.reserve(model->faceCount * 3);
    for (int i = 0; i < model->faceCount; i++) {
        const Face& face = model->faces[i];
        int v1 = face.v1 - 1;
        int v2 = face.v2 - 1;
        int v3 = face.v3 - 1;

        indices.push_back(v1);
        indices.push_back(v2);
        indices.push_back(v3);

        adjencyList[v1].push_back(v2); adjencyList[v2].push_back(v1);
        adjencyList[v2].push_back(v3); adjencyList[v3].push_back(v2);
        adjencyList[v3].push_back(v1); adjencyList[v1].push_back(v3);
    }
}

// Na sua função handleKeyboard, mude as chamadas:
void handleKeyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 27: exit(0); break;
        // Passe o endereço da câmera global para as funções
        case 'w': cameraMoveForward(&camera, 10); break;
        case 's': cameraMoveForward(&camera, -10); break;
        case 'a': cameraRotateY(&camera, -5); break;
        case 'd': cameraRotateY(&camera, 5);  break;
        case 'r': resetParticle(); break;
    }
    glutPostRedisplay();
}

std::string makeKey(const Vertex& v) {
    return std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z);
}

void processTerrain(ObjModel *model){
    vertices.clear(); //limpa toda estrutura que estava previamente armazenada
    adjencyList.clear();
    indices.clear();
    
    
    if (!model || model->vertexCount == 0) return;

    float centerX = (model->box.minX + model->box.maxX) / 2.0f;
    float centerY = (model->box.minY + model->box.maxY) / 2.0f;
    float centerZ = (model->box.minZ + model->box.maxZ) / 2.0f;

    float sizeX = model->box.maxX - model->box.minX;
    float sizeY = model->box.maxY - model->box.minY;
    float sizeZ = model->box.maxZ - model->box.minZ;
    float modelSize = std::max({sizeX, sizeY, sizeZ});

    float scaleFactor = 1.0f;
    if (modelSize > 0) {
        scaleFactor = 100.0f / modelSize; // o modelo vai ter cerca de 100 unidades de tamanho
    }

    // --- Processamento dos Vértices com Transformação ---
    vertices.reserve(model->vertexCount);
    for(int i = 0; i < model->vertexCount; i++){
        // Pega o vértice original
        float originalX = model->vertices[i].x;
        float originalY = model->vertices[i].y;
        float originalZ = model->vertices[i].z;

        // Aplica a translação e a escala
        float newX = (originalX - centerX) * scaleFactor; //pega a coordenada X original e subtrai do centro do modelo
        float newY = (originalY - centerY) * scaleFactor;
        float newZ = (originalZ - centerZ) * scaleFactor;

        // Adiciona o vértice JÁ TRANSFORMADO à nossa lista
        vertices.push_back({newX, newY, newZ});
    }
    //construir a lista de adjacencia
    adjencyList.assign(vertices.size(), std::vector<int>());
    indices.reserve(model->faceCount * 3);
    for(int i = 0; i < model->faceCount;i++){
        const Face& face = model->faces[i];

        //os indices do .obj comecam em 1, por isso subtrai de um 
        int v1 = face.v1 - 1;
        int v2 = face.v2 - 1;
        int v3 = face.v3 - 1;

        indices.push_back(v1);
        indices.push_back(v2);
        indices.push_back(v3);

        // Adiciona as conexões recíprocas para cada aresta do triângulo (v1-v2, v2-v3, v3-v1)
        adjencyList[v1].push_back(v2);
        adjencyList[v2].push_back(v1);

        adjencyList[v2].push_back(v3);
        adjencyList[v3].push_back(v2);

        adjencyList[v3].push_back(v1);
        adjencyList[v1].push_back(v3);
    }
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
    glEnableClientState(GL_VERTEX_ARRAY); //falar dessa call back glEnableClientState
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), vertices.data()); // glVertexPointer falar dessa tambem
    glColor3f(0.2f, 0.6f, 0.3f);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, indices.data()); //entender melhor essa aqui tambem
    glDisableClientState(GL_VERTEX_ARRAY);

    glPopMatrix();
}

void drawParticle(){
    //so desenha se tiver numa posicao valida
    if(particle.currentVertexIndex >= 0){
        glPushMatrix();
        glTranslatef(particle.x, particle.y, particle.z);
        glColor3f(1.0f, 0.0f, 0.0f);
        glutSolidSphere(3.0, 16, 16); //raio 3
        glPopMatrix();
    }
    
}



void particlePrintingInformation(int selectedIndex){
    if (selectedIndex < 0 || selectedIndex >= vertices.size()) 
        return;

    std::cout << std::fixed << std::setprecision(2);

    std::cout << "******************************************" << std::endl;
    std::cout << "escoamento iniciado!" << std::endl;
    std::cout << "  Vertice selecionado: [" << selectedIndex << "] | Altura: " << vertices[selectedIndex].y << std::endl;
    std::cout << "******************************************" << std::endl;
}
void movementInfo(int fromIndex, int toIndex){
    std::cout << "  Movendo: De [" << fromIndex << "] para [" << toIndex << "]" << std::endl;

}



void flowSimulation(int value){
    if (particle.currentVertexIndex < 0){
        glutTimerFunc(1200, flowSimulation, 0); // fica nesse loop para permitir novos cliques
        return; //evitar seg fault se 
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
    glutTimerFunc(600, flowSimulation, 0);
}


void setupMainCamera(int w, int h) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)w / (h ? h : 1), 1.0, 5000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    cameraApplyView(&camera);
}

void setupMinimapCamera() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Projeção ortogonal que enquadra a área do terreno procedural
    glOrtho(-WIDTH/2.0, WIDTH/2.0, -HEIGHT/2.0, HEIGHT/2.0, -1000.0, 1000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // Câmera fixa olhando de cima para baixo
    gluLookAt(0, 400, 0, 0, 0, 0, 0, 0, -1);
}



void display() {
    // Obtém as dimensões atuais da janela
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);

    // Limpa a tela inteira uma única vez
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // --- 1º PASSE: Desenha a cena principal (perspectiva) ---
    glViewport(0, 0, w, h); // Usa a janela inteira
    setupMainCamera(w, h);
    drawTerrain();
    drawParticle();

    // --- 2º PASSE: Desenha o minimapa (ortogonal) ---
    int minimapSize = h / 4; // Tamanho do minimapa (ex: 1/4 da altura da tela)
    int margin = 10;         // Margem do canto
    glViewport(w - minimapSize - margin, h - minimapSize - margin, minimapSize, minimapSize);
    
    // Limpa o buffer de profundidade para desenhar o mapa "por cima" de tudo
    glClear(GL_DEPTH_BUFFER_BIT);

    setupMinimapCamera();
    drawTerrain();
    drawParticle();

    glutSwapBuffers();
}

void reshape(int width, int height) {

    if (height == 0) 
        height = 1;
    glViewport(0, 0, width, height);
}

void handleMouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (vertices.empty()) return;

        // ==> CORREÇÃO: RECONFIGURA A CÂMERA PRINCIPAL ANTES DE PROJETAR <==
        int w = glutGet(GLUT_WINDOW_WIDTH);
        int h = glutGet(GLUT_WINDOW_HEIGHT);
        glViewport(0, 0, w, h);
        setupMainCamera(w, h);

        GLdouble modelview[16], projection[16];
        GLint viewport[4];
        glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
        glGetDoublev(GL_PROJECTION_MATRIX, projection);
        glGetIntegerv(GL_VIEWPORT, viewport);

        float min_dist_sq = -1.0f;
        int closest_vertex_index = -1;

        for (int i = 0; i < vertices.size(); ++i) {
            GLdouble screenX, screenY, screenZ;
            gluProject(vertices[i].x, vertices[i].y, vertices[i].z, modelview, projection, viewport, &screenX, &screenY, &screenZ);
            float mouse_y_gl = viewport[3] - y;
            float dx = x - screenX;
            float dy = mouse_y_gl - screenY;
            float dist_sq = dx * dx + dy * dy;
            if (closest_vertex_index == -1 || dist_sq < min_dist_sq) {
                min_dist_sq = dist_sq;
                closest_vertex_index = i;
            }
        }
        
        const float SELECTION_RADIUS = 20.0f;
        if (closest_vertex_index != -1 && sqrt(min_dist_sq) < SELECTION_RADIUS) {
            const Vertex& v = vertices[closest_vertex_index];
            particle.currentVertexIndex = closest_vertex_index;
            particle.x = v.x;
            particle.y = v.y;
            particle.z = v.z;
            particlePrintingInformation(closest_vertex_index);
            glutPostRedisplay();
        }
    }
}

int main(int argc, char **argv) {
    //vou pegar do usuario oq ele quer
    char choice;
    std::cout << "Escolha o modo:" << std::endl;
    std::cout << "Digite 1: Gerar terreno retangular" << std::endl;
    std::cout << "Digite 2: Importar modelo .obj" << std::endl;
    std::cout << "opcao: ";
    std::cin >> choice;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1200, 720);
    glutCreateWindow("Terrain Generation");

    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // ver oq essa linha faz tambem
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f); // Cor de fundo

    // Inicializa a câmera da biblioteca
    camera.px = 0; camera.py = 50; camera.pz = 200; // Posição inicial afastada
    camera.tx = 0; camera.ty = 0; camera.tz = 0;   // Olhando para a origem


    if(choice == '1') {
        generateProceduralTerrain();
    } else if(choice == '2'){
        std::string objFile, mtlFile;
        std::cout << "Digite o nome do arquivo .obj: ";
        std::cin >> objFile;

        std::cout << "Digite o nome do arquivo .mtl: ";
        std::cin >> mtlFile;

        if(!loadOBJ(objFile.c_str(), mtlFile.c_str(), &model)){
            printf("erro importando o objeto\n");
            return -1;
        }

        processTerrain(&model);
        freeObjModel(&model);
    } else {
        std::cout << "tchau" << std::endl;
        return -1;
    }


    resetParticle();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(handleKeyboard);
    glutMouseFunc(handleMouseClick);
    glutTimerFunc(1200, flowSimulation, 0);

    glutMainLoop();
    return 0;
}
