#include <cstdio>
#include <iostream>
#include <GL/glut.h>
#include <GL/glu.h>
#include <cmath>

// Початкові параметри
bool wireframe = false;     // режим відображення
bool perspective = true;    // тип проекції
bool useFlatShading = false; // Плоске або інтерполяційне зафарбування

// Render mode для NURBS (заливка, каркас тощо)
enum NurbsMode { FILL, OUTLINE_POLYGON, OUTLINE_PATCH };
NurbsMode renderMode = FILL;

// Матеріал/освітлення для NURBS-поверхні: базовий колір (можна міняти клавішею 'c')
GLfloat surfaceColor[3] = { 0.02f, 0.015f, 0.67f };

// Глобальні об'єкти GLU
GLUquadric* quad;
GLUnurbsObj* nurbs;

// Контрольні точки для бі-сплайнової поверхні (розмір 4x4)
GLfloat controlPoints[4][4][3];

// Knot vectors для поверхні (4x4 => 8 елементів)
GLfloat uKnot[] = { 0, 0, 0, 0, 1, 1, 1, 1 };
GLfloat vKnot[] = { 0, 0, 0, 0, 1, 1, 1, 1 };

struct transform {
    GLdouble yaw = 0.0f;
    GLdouble pitch = 20.0f;
    GLdouble zoom = 10.0f;
    int lastX = 0, lastY = 0;
    bool isDragging = false;
} SceneTransform;

// Анімування однієї контрольної точки (наприклад, [1][1])
// Зберігаємо її базове значення:
GLfloat baseZ = 3.0f;

// GLU_SAMPLING_TOLERANCE – можна змінювати клавішею 't'
GLfloat samplingTolerance = 25.0f;

//
// Ініціалізація
//
void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    // Налаштування матеріалу (блиск і specular)
    GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat mat_shininess[] = {50.0};
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    // Створення об'єктів GLU
    quad = gluNewQuadric();
    nurbs = gluNewNurbsRenderer();

    // Початкові параметри NURBS
    gluNurbsProperty(nurbs, GLU_DISPLAY_MODE, GLU_FILL);
    gluNurbsProperty(nurbs, GLU_SAMPLING_TOLERANCE, samplingTolerance);

    // Ініціалізація контрольних точок для бі-сплайнової поверхні
    for (int u = 0; u < 4; ++u) {
        for (int v = 0; v < 4; ++v) {
            // Простір від -1.5 до +1.5 по X та Y (масштаб 3.0)
            controlPoints[u][v][0] = 3.0f * (static_cast<float>(u) / 3.0f - 0.5f);
            controlPoints[u][v][1] = 3.0f * (static_cast<float>(v) / 3.0f - 0.5f);
            // В центрі піднята, решта на 0
            controlPoints[u][v][2] = (u == 1 && v == 1) ? baseZ : 0.0f;
        }
    }
}

//
// Малювання NURBS-поверхні з вирізаним отвором
//
void drawSurfaceWithHole() {
    // Встановлення матеріалу (колір)
    glColor3fv(surfaceColor);

    gluNurbsProperty(nurbs, GLU_SAMPLING_TOLERANCE, samplingTolerance);
    // Режим рендерингу можна перемикати
    switch (renderMode) {
        case FILL:
            gluNurbsProperty(nurbs, GLU_DISPLAY_MODE, GLU_FILL);
            break;
        case OUTLINE_POLYGON:
            gluNurbsProperty(nurbs, GLU_DISPLAY_MODE, GLU_OUTLINE_POLYGON);
            break;
        case OUTLINE_PATCH:
            gluNurbsProperty(nurbs, GLU_DISPLAY_MODE, GLU_OUTLINE_PATCH);
            break;
    }
    glShadeModel(useFlatShading ? GL_FLAT : GL_SMOOTH);

    gluBeginSurface(nurbs);

    gluNurbsSurface(nurbs,
        8, uKnot, 8, vKnot,
        4 * 3, 3,
        &controlPoints[0][0][0],
        4, 4,
        GL_MAP2_VERTEX_3);

    // === Outer boundary in (u,v), from (0,0) to (1,1) ===
    GLfloat outerPoints[10][2] = {
        {0.5f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 0.5f},
        {1.0f, 1.0f},
        {0.5f, 1.0f},
        {0.0f, 1.0f},
        {0.0f, 0.5f},
        {0.0f, 0.1f},
        {0.1f, 0.0f},
        {0.5f, 0.0f}
    };

    GLfloat outerKnot[] = {0, 0, 0, 1, 2, 3, 4, 5, 5, 6, 7, 7, 7};

    gluBeginTrim(nurbs);
    gluNurbsCurve(nurbs, 13, outerKnot, 2, &outerPoints[0][0], 3, GLU_MAP1_TRIM_2);
    gluEndTrim(nurbs);

    GLfloat holePoints[9][2] = {
        {0.2f, 0.2f},
        {0.02f, 0.4f},
        {0.2f, 0.5f},
        {0.3f, 0.6f},
        {0.2f, 0.7f},
        {0.4f, 0.7f},
        {0.5f, 0.7f},
        {0.8f, 0.3f},
        {0.2f, 0.2f},
    };
    // === Hole (inner trim loop) ===
    GLfloat holeKnot[] = {0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 7, 7};

    gluBeginTrim(nurbs);
    gluNurbsCurve(nurbs, 12, holeKnot, 2, &holePoints[0][0], 3, GLU_MAP1_TRIM_2);
    gluEndTrim(nurbs);

    gluEndSurface(nurbs);
}

//
// Малювання циліндра (зліва від поверхні)
//
void drawCylinder() {
    glColor3f(1.f, 0.75f, 0.f);
    if (wireframe)
        gluQuadricDrawStyle(quad, GLU_LINE);
    else
        gluQuadricDrawStyle(quad, GLU_FILL);

    glPushMatrix();
    glTranslatef(-2.0f, 0.0f, 0.0f);
    gluCylinder(quad, 1.0, 1.0, 3.0, 32, 32);
    glPopMatrix();
}

//
// Оновлення камери (орбітна камера)
//
void updateCamera() {
    glLoadIdentity();

    float camX = SceneTransform.zoom * cosf(SceneTransform.pitch * M_PI / 180.f) * sinf(SceneTransform.yaw * M_PI / 180.f);
    float camY = SceneTransform.zoom * sinf(SceneTransform.pitch * M_PI / 180.f);
    float camZ = SceneTransform.zoom * cosf(SceneTransform.pitch * M_PI / 180.f) * cosf(SceneTransform.yaw * M_PI / 180.f);

    gluLookAt(camX, camY, camZ,
              0.0, 0.0, 0.0,
              0.0, 1.0, 0.0);
}

//
// Анімаційна функція – оновлюється кожен кадр
//
void idleFunc() {
    // Використовуємо час для анімації (зміна контролюючої точки [1][1])
    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f; // секунди
    controlPoints[1][1][2] = baseZ + sinf(time) * 1.0f; // коливання ±1
    glutPostRedisplay();
}

//
// Малювання сцени
//
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    updateCamera();

    // Малюємо поверхню з отвором
    glPushMatrix();
    drawSurfaceWithHole();
    glPopMatrix();

    // Малюємо циліндр
    drawCylinder();

    glutSwapBuffers();
}

//
// Обробка зміни розміру вікна
//
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (perspective)
        gluPerspective(60.0, static_cast<float>(w) / h, 1.0, 100.0);
    else
        glOrtho(-3, 3, -3, 3, 1, 100);
    glMatrixMode(GL_MODELVIEW);
}

//
// Обробка клавіатури
//
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'l':
        case 'L':
            wireframe = !wireframe;
            break;
        case 'p':
        case 'P':
            perspective = !perspective;
            reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
            break;
        // Зміна кольору поверхні: натисни 'c'
        case 'c':
        case 'C': {
            // Проста ротація між кількома кольорами
            static int colIndex = 0;
            colIndex = (colIndex + 1) % 4;
            switch (colIndex) {
                case 0:
                    surfaceColor[0] = 0.02f; surfaceColor[1] = 0.015f; surfaceColor[2] = 0.67f; break;
                case 1:
                    surfaceColor[0] = 0.67f; surfaceColor[1] = 0.15f; surfaceColor[2] = 0.02f; break;
                case 2:
                    surfaceColor[0] = 0.02f; surfaceColor[1] = 0.67f; surfaceColor[2] = 0.15f; break;
                case 3:
                    surfaceColor[0] = 0.5f; surfaceColor[1] = 0.5f; surfaceColor[2] = 0.5f; break;
            }
            break;
        }
        // Зміна GLU_SAMPLING_TOLERANCE (натисни 't' для зменшення, 'T' для збільшення)
        case 't':
            samplingTolerance = (samplingTolerance > 1.0f) ? samplingTolerance - 1.0f : samplingTolerance;
            break;
        case 'T':
            samplingTolerance += 1.0f;
            break;
        // Перемикання режимів рендерингу для NURBS-поверхні (натисни 'm')
        case 'm':
        case 'M':
            renderMode = static_cast<NurbsMode>((renderMode + 1) % 3);
        break;
        case 'f':
        case 'F':
            useFlatShading = !useFlatShading;
        break;
        case 27:
            exit(0);
    }
    glutPostRedisplay();
}

//
// Обробка миші
//
void mouse(int button, int state, int x, int y) {
    if (button == 3 && state == GLUT_DOWN) { // scroll up
        SceneTransform.zoom -= 0.5f;
        if (SceneTransform.zoom < 2.f) SceneTransform.zoom = 2.f;
        glutPostRedisplay();
    } else if (button == 4 && state == GLUT_DOWN) { // scroll down
        SceneTransform.zoom += 0.5f;
        if (SceneTransform.zoom > 100.f) SceneTransform.zoom = 100.f;
        glutPostRedisplay();
    }
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            SceneTransform.isDragging = true;
            SceneTransform.lastX = x;
            SceneTransform.lastY = y;
        } else {
            SceneTransform.isDragging = false;
        }
    }
}

//
// Обробка руху миші (drag-to-rotate)
//
void motion(int x, int y) {
    if (SceneTransform.isDragging) {
        float dx = x - SceneTransform.lastX;
        float dy = y - SceneTransform.lastY;

        SceneTransform.yaw -= dx * 0.5f;
        SceneTransform.pitch += dy * 0.5f;
        if (SceneTransform.pitch > 89.0f) SceneTransform.pitch = 89.0f;
        if (SceneTransform.pitch < -89.0f) SceneTransform.pitch = -89.0f;

        SceneTransform.lastX = x;
        SceneTransform.lastY = y;
        glutPostRedisplay();
    }
}

//
// Main function
//
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Andrii's GLU Magic");

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idleFunc);  // Анімація поверхні

    glutMainLoop();
    return 0;
}
