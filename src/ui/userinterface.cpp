#include "userinterface.h"

UserInterface::UserInterface()
{
}

UserInterface::~UserInterface()
{
}

void UserInterface::initialize()
{
    m_stringDrawer.initialize();
}

void UserInterface::showHUD()
{
    m_stringDrawer.paint("Active element: wotör", glm::mat4(0.5, 0, 0, 0, 
                                                            0, 0.5, 0, 0, 
                                                            0, 0, 0.5, 0, 
                                                            -0.95, -0.95, 0, 1), StringDrawer::Alignment::kAlignLeft);
    drawPreview();
}

void UserInterface::showMainMenu()
{
}

void UserInterface::drawPreview()
{
    drawPreviewCircle(0.94, -0.95);
    drawPreviewCircle(0.83, -0.95);
    drawPreviewCircle(0.72, -0.95);
    drawPreviewCircle(0.61, -0.95);
}

void UserInterface::drawPreviewCircle(float x, float y)
{
    const int   POINTS = 3*360;
    const float RADIUS = 0.05;

    float points[POINTS];

    for (int i = 0; i < POINTS; i+=3)
    {
        double theta = i/float(POINTS)*3.14*2;
        points[i + 2] = 0;
        points[i + 1] = y + cosf(theta) * RADIUS;
        points[i + 0] = x + sinf(theta) * RADIUS;
    }

    unsigned int vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, POINTS * sizeof (float), points, GL_STATIC_DRAW);

    unsigned int vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    const char* vertex_shader =
        "#version 400\n"
        "in vec3 vp;"
        "void main () {"
        "  gl_Position = vec4 (vp, 1.0);"
        "}";

    const char* fragment_shader =
        "#version 400\n"
        "in vec3 vp;"
        "void main () {"
        "  gl_Position = vec4 (vp, 1.0);"
        "}";


    glBindVertexArray(vao);
    // draw points 0-3 from the currently bound VAO with current in-use shader
    glDrawArrays(GL_TRIANGLE_FAN, 0, POINTS/3);
}