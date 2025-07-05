#define GL_SILENCE_DEPRECATION //sileciar warnings no MacOS
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
#include <set>
#include <utility>

// para compilar importando o terreno: 
//  g++ terrainGenaration.cpp mybib.c -o terrain -framework OpenGL -framework GLUT 
//  vai dar um warning por conta da diferenca de c e c++

const float SPEED_PARTICLE = 0.02f;

const int WIDTH = 1400;
const int HEIGHT = 1000;
const int TERRAIN_SCALE = 20; //isso aqui eh pra ter uma especie de escala
const int COLUMNS = WIDTH/ TERRAIN_SCALE;
const int ROWS = HEIGHT / TERRAIN_SCALE;
int isWireframe = true;

struct Particle {
    int targetVertexIndex = -1;
    int currentVertexIndex = -1; // inicializa com -1 porque ainda não conhece o proximo destino válido para se mover
    float progress = 0.0f;
};

//struct criada para pegar todo o terreno da camera vista de cima- ver a funcao setUpTopDownCamera()

struct BoundingBox {
    float minX, maxX, minY, maxY, minZ, maxZ;
};

BoundingBox terrainBounds;
std::vector<Particle> particles; //vetor pra guardar as múltiplas particulas
Camera camera;
Particle particle;
std::vector<Vertex> vertices;
std::vector<int> indices;
std::vector<std::vector<int>> adjencyList; //cria uma lista de vertices, e pra cada um deles, tem uma lista dos vizinhos
//o indice da lista externa std::vector< ... > eh o indice do vertice que estamos consultando atualmente
//a lista interna std::vector<int>
// O índice da lista externa corresponde ao índice do vértice que estamos consultando
ObjModel model;


//estruturas para guardar os vertices que para pintar o caminho
std::set<int>traversedVertex; //conjunto dos indices de cada vertice
std::set<std::pair<int, int>> traversedEdges; //guarda dois numeros int que definem uma unica aresta. Aresta que liga vertice a ao b ficaria

//vetor de normais para visualizacao preenchida
std::vector<Vertex> normals;
void cameraApplyView(const Camera* cam) {
    gluLookAt(cam->px, cam->py, cam->pz, cam->tx, cam->ty, cam->tz, 0.0f, 1.0f, 0.0f);
}

void calculateBounds(){
    if (vertices.empty())
        return;
    
        terrainBounds.minX = terrainBounds.maxX = vertices[0].x;
        terrainBounds.minY = terrainBounds.maxY = vertices[0].y;
        terrainBounds.minZ = terrainBounds.maxZ = vertices[0].z;

        for(const auto& v: vertices){
            if (v.x < terrainBounds.minX)
                terrainBounds.minX = v.x;
            if (v.x > terrainBounds.maxX)
                terrainBounds.maxX = v.x;
            if (v.y < terrainBounds.minY)
                terrainBounds.minY = v.y;
            if (v.y > terrainBounds.maxY)
                terrainBounds.maxY = v.y;
            if (v.z < terrainBounds.minZ)
                terrainBounds.minZ = v.z;
            if (v.z > terrainBounds.maxZ)
                terrainBounds.maxZ = v.z;
        }
        
}

void calculateNormals() {
    if (vertices.empty() || indices.empty()) return;

    // inicializo um vetor de normais com o mesmo tamanho do de vértices, preenchido com zeros.
    normals.assign(vertices.size(), {0.0f, 0.0f, 0.0f});

    // itero por cada triângulo da malha.
    for (size_t i = 0; i < indices.size(); i += 3) {
        // pego os indices dos três vértices do triangulo.
        int i1 = indices[i];
        int i2 = indices[i+1];
        int i3 = indices[i+2];

        // pego as coordenadas dos vértices.
        const Vertex& v1 = vertices[i1];
        const Vertex& v2 = vertices[i2];
        const Vertex& v3 = vertices[i3];

        // Calcula os dois vetores da aresta do triângulo.
        Vertex edge1 = {v2.x - v1.x, v2.y - v1.y, v2.z - v1.z};
        Vertex edge2 = {v3.x - v1.x, v3.y - v1.y, v3.z - v1.z};

        // Calcula a normal da face usando o produto vetorial.
        Vertex faceNormal;
        faceNormal.x = edge1.y * edge2.z - edge1.z * edge2.y;
        faceNormal.y = edge1.z * edge2.x - edge1.x * edge2.z;
        faceNormal.z = edge1.x * edge2.y - edge1.y * edge2.x;

        // 3. Adiciona a normal da face a cada um dos três vértices do triângulo.
        normals[i1].x += faceNormal.x; 
        normals[i1].y += faceNormal.y; 
        normals[i1].z += faceNormal.z;
        normals[i2].x += faceNormal.x; 
        normals[i2].y += faceNormal.y; 
        normals[i2].z += faceNormal.z;
        normals[i3].x += faceNormal.x; 
        normals[i3].y += faceNormal.y; 
        normals[i3].z += faceNormal.z;
    }

    // 4. Normaliza todos os vetores normais dos vértices.
    for (auto& normal : normals) {
        float len = sqrt(normal.x *normal.x + normal.y *normal.y + normal.z *normal.z);
        if (len > 0.0f) {
            normal.x /= len;
            normal.y /= len;
            normal.z /= len;
        }
    }
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


// ================
// Geração de um terreno pelo código

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
    
    // arrasta o terreno para a origem para que a câmera1 funcione bem
    for (auto& v : vertices) {
        v.x -= WIDTH / 2.0f;
        v.z -= HEIGHT / 2.0f;
    }

    calculateNormals();
}


void handleKeyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 27: exit(0); break; // 27 eh o exit
        case 'w': cameraMoveForward(&camera, 10); break;
        case 's': cameraMoveForward(&camera, -10); break;
        case 'a': cameraRotateY(&camera, -5); break;
        case 'd': cameraRotateY(&camera, 5);  break;
        case 'r': resetParticle(); break;
        case 'f': 
            isWireframe = !isWireframe;
            if(isWireframe){
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                std::cout << "visualizacao: Wireframe" << std::endl;
            } else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                std::cout << "isualizacao: filled" << std::endl;
            }
    }
    glutPostRedisplay();
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
    //faco com que a maior dimensao fique 100
    // processa vertices em transformacao
    vertices.reserve(model->vertexCount);
    for(int i = 0; i < model->vertexCount; i++){
        // pega o vértice original
        float originalX = model->vertices[i].x;
        float originalY = model->vertices[i].y;
        float originalZ = model->vertices[i].z;

        // aplica a translação e a escala
        float newX = (originalX - centerX) * scaleFactor; //pega a coordenada X original e subtrai do centro do modelo
        float newY = (originalY - centerY) * scaleFactor;
        float newZ = (originalZ - centerZ) * scaleFactor;

        // Adiciona o vértice !!!JÁ TRANSFORMADO!!!! na lista de vertices
        vertices.push_back({newX, newY, newZ});
    }
    //construo a lista de adjacencia
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

        // adiciona as conexões recíprocas para cada aresta do triângulo (v1-v2, v2-v3, v3-v1)
        adjencyList[v1].push_back(v2);
        adjencyList[v2].push_back(v1);

        adjencyList[v2].push_back(v3);
        adjencyList[v3].push_back(v2);

        adjencyList[v3].push_back(v1);
        adjencyList[v1].push_back(v3);

        calculateNormals();
    }
}
void generateTerrain() {
    //primeiro gerar todos os vertices da grade
    vertices.reserve(COLUMNS * ROWS); //questao de performance, pede pra alocar memoria de tamanho COLUMNS * ROWS
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
    adjencyList.assign(vertices.size(), std::vector<int>()); // apague todo o conteúdo de adjacencyList. Depois, preencha adjacencyList com vertices.size()  cópias de um novo vetor de inteiros vazio.
    //o vetor indices armazena os "endereços" dos vértices que formam cada triângulo
    // cada um triangulo precisa de 3 indices para os vertices, e cada quadrado tem 2 triangulos
    //logo 2 * 3 indices por quadrado
    indices.reserve((ROWS - 1) * (COLUMNS - 1) * 6);

    //gerar os indices dos vertices e a lista de adj
    //to iterando sobre os quadrados
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
    
    // habilita o uso de arrays de vértices e normais
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    // Aponta para os dados dos vértices e das normais
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), vertices.data());
    glNormalPointer(GL_FLOAT, sizeof(Vertex), normals.data());

    // cor do material pra iluminação
    glColor3f(0.2f, 0.6f, 0.3f);
    
    // desenha triângulos
    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, indices.data());

    // Desabilita os arrays
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glPopMatrix();
}

void drawParticle(){
    //so desenha se tiver numa posicao valida
    if(particle.currentVertexIndex >= 0){
       const Vertex& startPosition = vertices[particle.currentVertexIndex];
       //pego a posicao de chegada
       const Vertex& endPosition = (particle.targetVertexIndex >= 0) ? vertices[particle.targetVertexIndex]: startPosition;

       //aqui uso a interpolacao linear, que basicamente eh
       // pos = startPos + (endPos - startPos) * progress;
       float px = startPosition.x + (endPosition.x - startPosition.x) * particle.progress;
       float py = startPosition.y + (endPosition.y - startPosition.y) * particle.progress;
       float pz = startPosition.z + (endPosition.z - startPosition.z) * particle.progress;

       glPushMatrix();
       glTranslatef(px, py, pz);
       glutSolidSphere(3.0, 16, 16);
       glPopMatrix();
    }
    
}



void particlePrintingInformation(int selectedIndex){
    if (selectedIndex < 0 || selectedIndex >= vertices.size()) 
        return;

    std::cout << std::fixed << std::setprecision(2);

    std::cout << "****" << std::endl;
    std::cout << "escoamento iniciado!" << std::endl;
    std::cout << "  Vertice selecionado: [" << selectedIndex << "] | Altura: " << vertices[selectedIndex].y << std::endl;
    std::cout << "****" << std::endl;
}
void movementInfo(int fromIndex, int toIndex){
    std::cout << "  Movendo: De [" << fromIndex << "] para [" << toIndex << "]" << std::endl;

}


void flowSimulation(int value){
    int needsRedisplay = 0;
    //animacao so ocorre se houver um alvo e nao chegou ao vertice destino ainda
    if (particle.currentVertexIndex != -1 ){

            //particula chegou ao destino?
            if(particle.progress >= 1.0f){
                particle.progress = 1.0f; // faco a particula parar no alvo

                if(particle.targetVertexIndex != -1){ //se chegou no alvo, este novo alvo eh o novo ponto de partida
                    particle.currentVertexIndex = particle.targetVertexIndex;
                }
                
                traversedVertex.insert(particle.currentVertexIndex);

                //agora, eu comeco a procurar o novo alvo a partir do novo ponto, caso houver
                //vejo se tem algum vizinho ligado aquela lista
                const auto& neighbors = adjencyList[particle.currentVertexIndex];
                int nextIndexToMove = -1;
                float minHeight = vertices[particle.currentVertexIndex].y;
                for(int index: neighbors){ //index eh o indice do vizinho, guardo ele tambem caso haja
                    if(vertices[index].y < minHeight){
                        minHeight = vertices[index].y;
                        nextIndexToMove =index;
                    }
                }

                if(nextIndexToMove != -1){ // se tiver o nextIndexToMove diferente de -1 significa que tem um outro vertice, no caso mais baixo
                    movementInfo(particle.currentVertexIndex, nextIndexToMove);    
                    particle.targetVertexIndex = nextIndexToMove;
                    particle.progress = 0.0f;

                    int vertexInitial = particle.currentVertexIndex;
                    int vertexEnd = nextIndexToMove;

                    //padronizo para a menor aresta ser sempre o menor
                    int lowerIndex = std::min(vertexInitial, vertexEnd);
                    int biggerIndex = std::max(vertexInitial, vertexEnd);

                    std::pair<int, int> traversedEdge = {lowerIndex, biggerIndex};

                    traversedEdges.insert(traversedEdge);


                } else{
                    particle.targetVertexIndex = -1;//particula nao encontrada, ja ta no minimo
                }

            }

            // Se a partícula está no meio de uma viagem, avança o progresso
            if (particle.targetVertexIndex != -1 && particle.currentVertexIndex != particle.targetVertexIndex) {
                particle.progress += SPEED_PARTICLE;
                needsRedisplay = 1;
            }

    }

    if(needsRedisplay){
        glutPostRedisplay();
    }

    glutTimerFunc(16, flowSimulation, 0); // atualizacao de 60 frames por segundo 
    
}


void drawEdgePaths(){
    glPushMatrix();
    glLineWidth(2.8f); //desenha a linha
    glColor3f(1.0f, 1.0f, 0.0f); //cor amarela
    glBegin(GL_LINES);
    for(const auto& edge: traversedEdges){
        glVertex3fv(&vertices[edge.first].x);
        glVertex3fv(&vertices[edge.second].x);
    }
    glEnd();
    glLineWidth(1.0f);
    glPopMatrix();
}

void drawPathVertices(){
    glPushMatrix();
    glPointSize(5.0f);
    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_POINTS);
    for( int i: traversedVertex){
        glVertex3fv(&vertices[i].x);
    }

    glEnd();
    glPointSize(1.0f);
    glPopMatrix();
}
void setupMainCamera(int w, int h) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)w / (h ? h : 1), 1.0, 5000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    cameraApplyView(&camera);
}

void setUpTopDownCamera(){
    //void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float centerX = (terrainBounds.minX + terrainBounds.maxX) /2.0f;
    float centerZ = (terrainBounds.minZ + terrainBounds.maxZ) /2.0f;

    float sizeX = terrainBounds.maxX - terrainBounds.minX;
    float sizeZed = terrainBounds.maxZ - terrainBounds.minZ;

    float maxDimension = std::max(sizeX, sizeZed);
    float halfView = (maxDimension/2.0f) * 1.1f;

    glOrtho(centerX - halfView, centerX + halfView ,centerZ - halfView, centerZ + halfView, -500, 500);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(centerX, 400, centerZ, centerX,0, centerZ,0,0,-1);
}

void setupMinimapCamera() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // calcula o centro do terreno nos eixos que nos interessam para esta vista (Y e Z)
    float centerY = (terrainBounds.minY + terrainBounds.maxY) / 2.0f;
    float centerZ = (terrainBounds.minZ + terrainBounds.maxZ) / 2.0f;
    
    // calcula o tamanho real do perfil do terreno (altura e profundidade)
    float sizeY = terrainBounds.maxY - terrainBounds.minY;
    float sizeZ = terrainBounds.maxZ - terrainBounds.minZ;

    // adiciona uma margem de 10% (para não ficar colado nas bordas)
    float halfHeight = (sizeY / 2.0f) * 1.1f;
    float halfWidth = (sizeZ / 2.0f) * 1.1f;
    
    // garante que a vista não fique totalmente achatada se o terreno for plano
    if (halfHeight < 1.0f) halfHeight = 50.0f;

    // usando as proporções reais do modelo
    // A largura da vista (left/right) corresponde à profundidade Z do terreno
    // A altura da vista (bottom/top) corresponde à altura Y do terreno
    glOrtho(centerZ - halfWidth, centerZ + halfWidth, centerY - halfHeight, centerY + halfHeight, -2000.0, 2000.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // coloco a câmera na lateral, olhando para o centro 
    float cameraDistance = terrainBounds.maxX + (terrainBounds.maxX - terrainBounds.minX);
    gluLookAt(cameraDistance, centerY, centerZ, 
              0, centerY, centerZ, // Olha para o centro Y/Z do objeto, mas no plano x=0
              0, 1, 0);
}


void display() {
    // pego as dimensões atuais da janela
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);

    // limpo a tela inteira uma única vez
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //desenho a cena principal
    glViewport(0, 0, w, h); //janela inteira
    setupMainCamera(w, h);
    drawTerrain();
    drawEdgePaths();
    drawPathVertices();
    drawParticle();

    // ---viewport lateral
    int minimapSize = h / 4 ; // tamanho do minimapa (ex: 1/4 da altura da tela)
    int margin = 10;         // margem do canto
    glViewport(w - minimapSize - margin, h - minimapSize - margin, minimapSize, minimapSize);
    // Limpa o buffer 
    glClear(GL_DEPTH_BUFFER_BIT);

    setupMinimapCamera();
    drawTerrain();
    drawEdgePaths();
    drawPathVertices();
    drawParticle();

    //visao de cima
    glViewport(margin, h - minimapSize - margin, minimapSize, minimapSize);
    glClear(GL_DEPTH_BUFFER_BIT);
    setUpTopDownCamera();
    drawTerrain();
    drawEdgePaths();
    drawPathVertices();
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

    
        int w = glutGet(GLUT_WINDOW_WIDTH);
        int h = glutGet(GLUT_WINDOW_HEIGHT);
        glViewport(0, 0, w, h);
        setupMainCamera(w, h);

        //pego as informacoes 
        //modelview:pego informacao de posicao e orientacao da camera
        //projection:pego a informacao da camera1 - a principal
        GLdouble modelview[16], projection[16];
        GLint viewport[4];
        glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
        glGetDoublev(GL_PROJECTION_MATRIX, projection);
        glGetIntegerv(GL_VIEWPORT, viewport);

        float min_dist_sq = -1.0f; //pego o vertice mais proximo
        int closest_vertex_index = -1;

        for (int i = 0; i < vertices.size(); ++i) {
            GLdouble screenX, screenY, screenZ;
            //projeto o ponto para tridimensional para a tela 2D
            gluProject(vertices[i].x, vertices[i].y, vertices[i].z, modelview, projection, viewport, &screenX, &screenY, &screenZ);
            float mouse_y_gl = viewport[3] - y;//converto a coordenada y 
            float dx = x - screenX;
            float dy = mouse_y_gl - screenY;
            float dist_sq = dx * dx + dy * dy;//calcula a distancia entre o ponto e o vertice
            //verifico se eh o ponto mais proximo ate agora
            if (closest_vertex_index == -1 || dist_sq < min_dist_sq) {
                min_dist_sq = dist_sq;
                closest_vertex_index = i;
            }
        }
        //closest_vertex_index contem o indice do vertice que esta visualmente mais proximo de
        //oonde foi clicado
        const float SELECTION_RADIUS = 20.0f;
        //verifico a distancia eh menor que o raio(o nosso erro)
        //evito que um clique no nada faca com q ocorra algo
        if (closest_vertex_index != -1 && sqrt(min_dist_sq) < SELECTION_RADIUS) {
            traversedVertex.clear();
            traversedEdges.clear();

            //ponto de partida e ponto de chegada sao iguais
            particle.currentVertexIndex = closest_vertex_index;
            particle.targetVertexIndex = closest_vertex_index;
            particle.progress = 1.0f; //comeca parada
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

    glEnable(GL_DEPTH_TEST); // ligo z buffer

    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat light_pos[] = { 1.0f, 1.0f, 1.0f, 0.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Define o modo inicial como wireframe
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f);

    // inicializa a câmera da biblioteca
    camera.px = 0; camera.py = 150; camera.pz = 350; // posição inicial afastada
    camera.tx = 0; camera.ty = 0; camera.tz = 0;   // olhando para a origem


    if(choice == '1') {
        generateProceduralTerrain();
    } else if(choice == '2'){
        std::string objFile;
        std::cout << "Digite o nome do arquivo .obj: ";
        std::cin >> objFile;

        if(!loadOBJ(objFile.c_str(), "", &model)){
            printf("erro importando o objeto\n");
            return -1;
        }

        processTerrain(&model);
        freeObjModel(&model);
    } else {
        std::cout << "tchau" << std::endl;
        return -1;
    }


    calculateBounds(); // adaptacao para visualizar de cima independente do tamanho do modelo
    resetParticle();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(handleKeyboard);
    glutMouseFunc(handleMouseClick);
    glutTimerFunc(16, flowSimulation, 0);

    glutMainLoop();
    return 0;
}
