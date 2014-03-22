#pragma once
#include "glm/glm.hpp"
#include "glow/ref_ptr.h"

struct GLFWwindow;

namespace glow{
    class VertexArrayObject;
    class Program;
}
class CameraEx;

class Rain{
public:
    Rain();
    ~Rain();

    void initialize();
    void draw(const CameraEx & camera);
    void resize(int width, int height);
    
protected:
    glow::ref_ptr<glow::VertexArrayObject> m_vao;
    glow::ref_ptr<glow::Program>           m_rainProgram;
    glm::vec2                              m_viewport;

private:
    Rain(const Rain&) = delete;
    void operator=(const Rain&) = delete;
};