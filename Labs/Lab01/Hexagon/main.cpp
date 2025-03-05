#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <sstream>
#include <iomanip>

const int NUM_HEXAGONS = 5;

struct Hexagon {
    float x, y;
    float size;
    float r, g, b;
};

Hexagon hexagons[NUM_HEXAGONS];

void generateHexagons() {
    srand(time(0));
    for (int i = 0; i < NUM_HEXAGONS; i++) {
        hexagons[i].x = (rand() % 400) / 200.0f - 1.0f;
        hexagons[i].y = (rand() % 400) / 200.0f - 1.0f;
        hexagons[i].size = 0.1f + static_cast<float>(rand()) / RAND_MAX * 0.5f;
        hexagons[i].r = static_cast<float>(rand()) / RAND_MAX;
        hexagons[i].g = static_cast<float>(rand()) / RAND_MAX;
        hexagons[i].b = static_cast<float>(rand()) / RAND_MAX;
    }
}

void drawHexagon(float x, float y, float size, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 6; i++) {
        float angle = 2.0f * 3.1415926f * float(i) / 6.0f;
        float dx = size * cos(angle);
        float dy = size * sin(angle);
        glVertex2f(x + dx, y + dy);
    }
    glEnd();
}

// Відображення тексту (залежить від GLUT)
void renderText(float x, float y, const std::string& text) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }
}

void render(int recursionRate = NUM_HEXAGONS) {
    glClear(GL_COLOR_BUFFER_BIT);

    // Спочатку малюємо шестикутники
    for (int i = 0; i < recursionRate; i++) {
        drawHexagon(hexagons[i].x, hexagons[i].y, hexagons[i].size, hexagons[i].r, hexagons[i].g, hexagons[i].b);

        std::ostringstream colorText;
        colorText << std::fixed << std::setprecision(2)
                  << hexagons[i].r << ", " << hexagons[i].g << ", " << hexagons[i].b;

        // Протилежний колір
        float textR = 1.0f - hexagons[i].r;
        float textG = 1.0f - hexagons[i].g;
        float textB = 1.0f - hexagons[i].b;
        glColor3f(textR, textG, textB);

        renderText(hexagons[i].x - 0.05f, hexagons[i].y, colorText.str());
    }

    glFlush();
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(500, 500, "GLEW + GLFW Hexagons", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW!" << std::endl;
        return -1;
    }

    int argc = 0;
    char** argv = nullptr;
    glutInit(&argc, argv);

    glClearColor(0, 0, 0, 1);
    generateHexagons();

    int renderRate = NUM_HEXAGONS;
    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {

        render(renderRate);
        glfwSwapBuffers(window);
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            generateHexagons();
            render();
            renderRate = NUM_HEXAGONS;
        }

        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {

            while (renderRate > 0){
                double currentTime = glfwGetTime();
                double deltaTime = currentTime - lastTime;

                if (deltaTime > 1.f) {
                    render(--renderRate);
                    glfwSwapBuffers(window);
                    glfwPollEvents();
                    lastTime = currentTime;
                }
            }
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
