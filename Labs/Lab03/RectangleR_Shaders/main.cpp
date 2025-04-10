#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>

// GLSL вершинний шейдер
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
uniform float angle;
uniform vec2 translation;
uniform vec2 pivot;
void main() {
    vec2 pos = aPos - pivot;
    float s = sin(angle);
    float c = cos(angle);
    vec2 rotated = vec2(pos.x * c - pos.y * s, pos.x * s + pos.y * c) + pivot;
    gl_Position = vec4(rotated + translation, 0.0, 1.0);
}
)";

// GLSL фрагментний шейдер
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}
)";

struct Vertex {
    float x, y;
};

float angle = 0.f;
float rotationSpeed = 0.05f;
float speed = 0.0002f;
float slope = 0.1f;
float positionX = 0.0f, positionY = 0.0f;
Vertex rotationVertex;
GLuint VAO, VBO, EBO, shaderProgram;

std::vector<Vertex> rectangle = {
    {-0.2f, -0.2f},
    { 0.2f, -0.2f},
    { 0.2f,  0.2f},
    {-0.2f,  0.2f},
};

GLuint indices[] = { 0, 1, 2, 2, 3, 0 };

void compileShader(GLuint shader, const char* source) {
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader Compilation Error:\n" << infoLog << std::endl;
    }
}

void setupShaders() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    compileShader(vertexShader, vertexShaderSource);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    compileShader(fragmentShader, fragmentShaderSource);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader Program Linking Error:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void setupBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, rectangle.size() * sizeof(Vertex), rectangle.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void updatePosition() {
    positionX += speed;
    positionY = slope * positionX;
    angle += rotationSpeed;

    if (positionX > 1.f || positionX < -0.8f) {
        speed = -speed;
        rotationSpeed = -rotationSpeed;
    }
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgram);

    GLuint angleLoc = glGetUniformLocation(shaderProgram, "angle");
    GLuint translationLoc = glGetUniformLocation(shaderProgram, "translation");
    GLuint pivotLoc = glGetUniformLocation(shaderProgram, "pivot");

    glUniform1f(angleLoc, angle * M_PI / 180.0f);
    glUniform2f(translationLoc, positionX, positionY);
    glUniform2f(pivotLoc, rotationVertex.x, rotationVertex.y);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glFlush();
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(600, 600, "Rotating Quad (Shaders)", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewInit();

    glClearColor(1, 1, 1, 1);
    rotationVertex = rectangle[0];

    setupShaders();
    setupBuffers();

    while (!glfwWindowShouldClose(window)) {
        updatePosition();
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) rotationVertex = rectangle[0];
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) rotationVertex = rectangle[1];
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) rotationVertex = rectangle[2];
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) rotationVertex = rectangle[3];
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
