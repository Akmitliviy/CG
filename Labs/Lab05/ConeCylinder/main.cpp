#include <GL/glut.h>
#include <cmath>
#include <iostream>
#include <ostream>

#define TEX_WIDTH 64
#define TEX_HEIGHT 64
GLubyte checkerTexture[TEX_HEIGHT][TEX_WIDTH][3]; // 3 канали (RGB)
GLuint textureID;


float rotationAngle = 0.0f;


struct transform {
    GLdouble yaw = 0.0f;
    GLdouble pitch = 20.0f;
    GLdouble zoom = 10.0f;
    int leftX = 0, leftY = 0;
    int rightX = 0, rightY = 0;
    int dx = 0, dy = 1;
    bool isDraggingLeft = false;
    bool isDraggingRight = false;
    float camX = 3;
    float camY = 3;
    float camZ = 5;
    float panOffsetX = 0.0f;
    float panOffsetY = 0.0f;
    float panOffsetZ = 0.0f;
} SceneTransform;

bool ambient = true;
bool directional = true;
bool point = true;
bool spot = true;

float pointLightX = 2.0f;
float spotLightX = 0.0f;

bool enableFog = true;
GLint fogMode = GL_LINEAR; // Можна змінювати на GL_EXP або GL_EXP2
GLfloat fogColor[4] = {0.1f, 0.1f, 0.1f, 1.0f};

void generateCheckerTexture() {
    for (int i = 0; i < TEX_HEIGHT; i++) {
        for (int j = 0; j < TEX_WIDTH; j++) {
            int checker = ((i / 8) % 2) ^ ((j / 8) % 2); // чергуємо квадрати
            GLubyte color = checker ? 255 : 0; // чорний/білий
            checkerTexture[i][j][0] = color;
            checkerTexture[i][j][1] = color;
            checkerTexture[i][j][2] = color;
        }
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEX_WIDTH, TEX_HEIGHT, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, checkerTexture);
}

void initLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL); // Дозволяє фарбувати через glColor
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glEnable(GL_NORMALIZE); // Нормалізує нормалі для коректного освітлення
}

void initFog() {
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, fogMode); // Лінійний або експоненційний
    glFogfv(GL_FOG_COLOR, fogColor); // Колір туману — як фон
    glFogf(GL_FOG_DENSITY, 0.4f); // Для EXP/EXP2
    glHint(GL_FOG_HINT, GL_NICEST); // Найвища якість
    glFogf(GL_FOG_START, 500.0f);     // Для лінійного туману
    glFogf(GL_FOG_END, 1000.0f);      // Для лінійного туману
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Темний фон

    generateCheckerTexture();
    glEnable(GL_TEXTURE_2D); // Увімкнути текстури

    initFog();

    initLighting();
}

void setupAmbientLight() {
    if (ambient) {
        GLfloat ambientColor[] = { 0.3f, 0.3f, 0.3f, 1.0f };
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);
    } else {
        GLfloat ambientColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);
    }
}


void setupDirectionalLight() {
    if (directional) {
        glEnable(GL_LIGHT0);
        GLfloat lightColor[] = { 0.9f, 0.9f, 0.9f, 1.0f };
        GLfloat lightDir[] = { -1.0f, -1.0f, -1.0f, 0.0f };

        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
        glLightfv(GL_LIGHT0, GL_POSITION, lightDir);
    } else {
        glDisable(GL_LIGHT0);
    }
}

void setupPointLight() {
    if (point) {
        glEnable(GL_LIGHT1);
        GLfloat lightColor[] = { 1.0f, 0.8f, 0.8f, 1.0f };
        GLfloat lightPos[] = { pointLightX, 3.0f, 2.0f, 1.0f };

        glLightfv(GL_LIGHT1, GL_DIFFUSE, lightColor);
        glLightfv(GL_LIGHT1, GL_POSITION, lightPos);

        // Реалістичне загасання (наприклад, до 50 юнітів)
        glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.09f);
        glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.032f);
    } else {
        glDisable(GL_LIGHT1);
    }
}


void setupSpotlight() {
    if (spot) {
        glEnable(GL_LIGHT2);
        GLfloat lightColor[] = { 1.0f, 1.0f, 0.6f, 1.0f };
        GLfloat lightPos[] = { spotLightX, 3.0f, 0.0f, 1.0f };
        GLfloat spotDir[] = { 0.0f, -1.0f, 0.0f };

        glLightfv(GL_LIGHT2, GL_DIFFUSE, lightColor);
        glLightfv(GL_LIGHT2, GL_POSITION, lightPos);
        glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, spotDir);
        glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 30.0f);
        glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 10.0f);

        // Більш реалістичне загасання
        glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.09f);
        glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.032f);
    } else {
        glDisable(GL_LIGHT2);
    }
}


void drawCone() {
    GLUquadric* quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);

    glPushMatrix();
    glColor3f(1.0f, 0.0f, 0.0f); // Червоний
    glRotatef(-90, 1, 0, 0); // Щоб стояв вертикально
    gluCylinder(quad, 1.0, 0.0, 2.0, 32, 32);
    glPopMatrix();

    gluDeleteQuadric(quad);
}

void drawCylinder() {
    GLUquadric* quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluQuadricTexture(quad, GL_TRUE); // Дозволити текстуру

    glPushMatrix();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glColor3f(1.0f, 1.0f, 1.0f);
    glTranslatef(0.0f, 1.0f, -1.0f);
    glRotatef(90, 0, 0, 1);

    gluCylinder(quad, 0.2, 0.2, 2.0, 32, 32);

    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
    gluDeleteQuadric(quad);
}



void updateCamera() {
    glLoadIdentity();

    float centerX = SceneTransform.camX;
    float centerY = SceneTransform.camY;
    float centerZ = SceneTransform.camZ;

    float targetX = SceneTransform.dx;
    float targetY = SceneTransform.dy;
    float targetZ = 0.0f;

    // Враховуємо панорамування
    centerX += SceneTransform.panOffsetX;
    centerY += SceneTransform.panOffsetY;
    centerZ += SceneTransform.panOffsetZ;
    targetX += SceneTransform.panOffsetX;
    targetY += SceneTransform.panOffsetY;
    targetZ += SceneTransform.panOffsetZ;

    gluLookAt(centerX, centerY, centerZ,
              targetX, targetY, targetZ,
              0.0, 1.0, 0.0);
}


void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    updateCamera();

    setupAmbientLight();
    setupDirectionalLight();
    setupPointLight();
    setupSpotlight();

    glRotatef(rotationAngle, 0.0f, 1.0f, 0.0f); // Обертання об’єкта

    drawCone();
    drawCylinder();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
        gluPerspective(60.0, static_cast<float>(w) / h, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void timer(int value) {
    rotationAngle += 0.5f;
    if (rotationAngle > 360.0f)
        rotationAngle -= 360.0f;

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0); // ~60 FPS
}

void updateCoordinates() {
    float camX = SceneTransform.zoom * cosf(SceneTransform.pitch * M_PI / 180.f) * sinf(SceneTransform.yaw * M_PI / 180.f);
    float camY = SceneTransform.zoom * sinf(SceneTransform.pitch * M_PI / 180.f);
    float camZ = SceneTransform.zoom * cosf(SceneTransform.pitch * M_PI / 180.f) * cosf(SceneTransform.yaw * M_PI / 180.f);

    SceneTransform.camX = camX;
    SceneTransform.camY = camY;
    SceneTransform.camZ = camZ;

    SceneTransform.dx = 0.0f; // Центр обертання — (0, 0, 0)
    SceneTransform.dy = 0.0f;
}



//
// Обробка миші
//
void mouse(int button, int state, int x, int y) {
    if (button == 3 && state == GLUT_DOWN) { // scroll up
        SceneTransform.zoom -= 0.5f;
        if (SceneTransform.zoom < 2.f) SceneTransform.zoom = 2.f;
        updateCoordinates();
        glutPostRedisplay();
    } else if (button == 4 && state == GLUT_DOWN) { // scroll down
        SceneTransform.zoom += 0.5f;
        if (SceneTransform.zoom > 100.f) SceneTransform.zoom = 100.f;
        updateCoordinates();
        glutPostRedisplay();
    }
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            SceneTransform.isDraggingLeft = true;
            SceneTransform.leftX = x;
            SceneTransform.leftY = y;
        } else {
            SceneTransform.isDraggingLeft = false;
        }
    }else if (button == GLUT_RIGHT_BUTTON) {
        if (state == GLUT_DOWN) {
            SceneTransform.isDraggingRight = true;
            SceneTransform.rightX = x;
            SceneTransform.rightY = y;
        } else {
            SceneTransform.isDraggingRight = false;
        }
    }
}

//
// Обробка руху миші (drag-to-rotate)
//
void motion(int x, int y) {
    if (SceneTransform.isDraggingLeft) {
        float dx = x - SceneTransform.leftX;
        float dy = y - SceneTransform.leftY;

        SceneTransform.yaw -= dx * 0.5f;
        SceneTransform.pitch += dy * 0.5f;
        if (SceneTransform.pitch > 89.0f) SceneTransform.pitch = 89.0f;
        if (SceneTransform.pitch < -89.0f) SceneTransform.pitch = -89.0f;

        SceneTransform.leftX = x;
        SceneTransform.leftY = y;

        updateCoordinates();
        glutPostRedisplay();
    }
    else if (SceneTransform.isDraggingRight) {
        float dx = x - SceneTransform.rightX;
        float dy = y - SceneTransform.rightY;

        float panSpeed = 0.005f * SceneTransform.zoom;

        // yaw у радіанах
        float yawRad = SceneTransform.yaw * M_PI / 180.0f;

        // Вектор вправо (по yaw)
        float rightX = cosf(yawRad);
        float rightZ = -sinf(yawRad);

        // Вектор вгору — простіше взяти по Y (можна ускладнити через pitch, але поки не треба)
        float upY = 1.0f;

        // Зсув у світових координатах
        SceneTransform.panOffsetX += -dx * panSpeed * rightX;
        SceneTransform.panOffsetZ += -dx * panSpeed * rightZ;
        SceneTransform.panOffsetY += dy * panSpeed * upY;

        SceneTransform.rightX = x;
        SceneTransform.rightY = y;

        glutPostRedisplay();
    }
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'r':
            SceneTransform.panOffsetX = 0.0f;
            SceneTransform.panOffsetY = 0.0f;
            SceneTransform.panOffsetZ = 0.0f;
            SceneTransform.camX = 3.0f;
            SceneTransform.camX = 3.0f;
            SceneTransform.camX = 10.0f;
            SceneTransform.yaw = 0.0f;
            SceneTransform.pitch = 20.0f;
            SceneTransform.zoom = 10.0f;

            updateCoordinates();
        break;
        case 'a':
            ambient = !ambient;
            std::cout << "ambient = " << ambient << std::endl;
        break;
        case 'd':
            directional = !directional;
            std::cout << "directional = " << directional << std::endl;
        break;
        case 'p':
            point = !point;
            std::cout << "point = " << point << std::endl;
        break;
        case 's':
            spot = !spot;
            std::cout << "spot = " << spot << std::endl;
        break;
        case 'f':
            enableFog = !enableFog;
        if (enableFog) glEnable(GL_FOG);
        else glDisable(GL_FOG);
        std::cout << "Fog: " << (enableFog ? "ON" : "OFF") << std::endl;
        break;

        case 'm':
            if (fogMode == GL_LINEAR)
                fogMode = GL_EXP;
            else if (fogMode == GL_EXP)
                fogMode = GL_EXP2;
            else
                fogMode = GL_LINEAR;

        glFogi(GL_FOG_MODE, fogMode);
        std::cout << "Fog mode: " << (fogMode == GL_LINEAR ? "LINEAR" : (fogMode == GL_EXP ? "EXP" : "EXP2")) << std::endl;
        break;
        case 27:
            exit(0);
    }
    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT:
            pointLightX -= 0.2f;
        spotLightX -= 0.2f;
        break;
        case GLUT_KEY_RIGHT:
            pointLightX += 0.2f;
        spotLightX += 0.2f;
        break;
    }
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Cone + Cylinder Base Scene");

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(0, timer, 0);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);

    glutMainLoop();
    return 0;
}
