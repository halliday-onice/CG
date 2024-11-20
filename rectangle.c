#include <GL/glew.h>   // Include GLEW before GLFW
#include <GLFW/glfw3native.h>
#include <stdio.h>
#include <stdlib.h>



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int main() {
    // Inicializa o GLFW
    if (!glfwInit()) {
        printf("Falha ao inicializar o GLFW\n");
        return -1;
    }

    // Define a versão do OpenGL (3.3) e o perfil core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Cria a janela
    GLFWwindow* window = glutCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Rectangle", NULL, NULL);
    if (!window) {
        printf("Falha ao criar a janela GLFW\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Inicializa o GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        printf("Falha ao inicializar o GLEW\n");
        return -1;
    }

    // Define a função de callback para redimensionamento da janela
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Vertices do retângulo
    float vertices[] = {
        // Posições        // Cores
        0.5f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // superior direito
        0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // inferior direito
       -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  // inferior esquerdo
       -0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 0.0f   // superior esquerdo
    };
    unsigned int indices[] = {
        0, 1, 3,   // primeiro triângulo
        1, 2, 3    // segundo triângulo
    };

    // Buffer de Vértices (VBO), Buffer de Índices (EBO), e Array de Vértices (VAO)
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // VBO para os vértices
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // EBO para os índices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Atributos de posição
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Atributos de cor
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Loop principal
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // Limpa a tela
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Desenha o retângulo
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Troca os buffers
        glfwSwapBuffers(window);

        // Processa eventos
        glfwPollEvents();
    }

    // Limpa os recursos
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}

// Redimensionamento da janela
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Processa a entrada do teclado
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, 1);
}
