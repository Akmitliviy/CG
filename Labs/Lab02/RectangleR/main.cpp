#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

struct Vertex {
    float x, y;
};

float angle = 0.05f;
float speed = 0.0002f;
float slope = 0.5f;
Vertex rotationVertex;

Vertex rectangle[] = {
    Vertex{-0.2f, -0.2f},
    Vertex{0.2f, -0.2f},
    Vertex{0.2f, 0.2f},
    Vertex{-0.2f, 0.2f},
};

void updateRectangle(float angleRad, float dx) {
    for (int i = 0; i < 4; i++) {
        // Рух уздовж прямої y = ax
        rectangle[i].x += dx;
        rectangle[i].y += slope * dx;
    }
    rotationVertex.x += dx;
    rotationVertex.y += slope * dx;
    for (int i = 0; i < 4; i++) {
        float rel_x = rectangle[i].x - rotationVertex.x;
        float rel_y = rectangle[i].y - rotationVertex.y;

        float rotated_x = rel_x * cos(angleRad) - rel_y * sin(angleRad);
        float rotated_y = rel_x * sin(angleRad) + rel_y * cos(angleRad);

        rectangle[i].x = rotated_x + rotationVertex.x;
        rectangle[i].y = rotated_y + rotationVertex.y;
    }
}

void drawQuad() {
    glBegin(GL_QUADS);
    glColor3f(0.0f, 1.0f, 0.0f);
    for (int i = 0; i < 4; i++) {
        glVertex2f(rectangle[i].x, rectangle[i].y);
    }
    glEnd();
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad();
    glFlush();
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(600, 600, "Rotating Quad", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewInit();

    glClearColor(1, 1, 1, 1);
    rotationVertex = rectangle[0];

    while (!glfwWindowShouldClose(window)) {
        float angleRad = angle * M_PI / 180.0f;
        updateRectangle(angleRad, speed);
        for (int i = 0; i < 4; i++) {
            if (rectangle[i].x < -0.8f || rectangle[i].x > 0.8f
                || rectangle[i].y < -0.8f || rectangle[i].y > 0.8f) {
                speed = -speed;
                angle = -angle;
            }
        }

        render();
        glfwSwapBuffers(window);
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) rotationVertex = rectangle[0];
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) rotationVertex = rectangle[1];
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) rotationVertex = rectangle[2];
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) rotationVertex = rectangle[3];
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}