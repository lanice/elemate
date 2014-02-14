/* This document is based on a file by Max Jendruk (Project Mammut) serving the same purpose but using Qt. */
#pragma once

#include <vector>

#include <glm/glm.hpp>

#include <glow/ref_ptr.h>

namespace glow
{
    class VertexArrayObject;
    class Buffer;
}

class CharacterDrawable
{
public:
    CharacterDrawable();
    virtual ~CharacterDrawable();

    void initialize();
    void draw();

protected:
    static const std::vector<glm::vec2> triangleStrip();
    
protected:
    glow::ref_ptr<glow::VertexArrayObject> m_vao;
    glow::ref_ptr<glow::Buffer> m_buffer;

};
