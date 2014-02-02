#include "CharacterDrawable.h"

#include <glow/VertexArrayObject.h>
#include <glow/VertexAttributeBinding.h>
#include <glow/Buffer.h>

CharacterDrawable::CharacterDrawable()
{
}

CharacterDrawable::~CharacterDrawable()
{
}

const glow::Vec2Array CharacterDrawable::triangleStrip()
{
    return glow::Vec2Array {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 1.0f)
    };
}

void CharacterDrawable::initialize()
{
    m_vao = new glow::VertexArrayObject();
    
    m_vao->bind();
    
    m_buffer = new glow::Buffer();
    m_buffer->bind(GL_ARRAY_BUFFER);
    m_buffer->setData(triangleStrip());
    
    glow::VertexAttributeBinding * vertices = m_vao->binding(0);
    vertices->setBuffer(m_buffer.get(), 0, sizeof(glm::vec2));
    vertices->setFormat(2, GL_FLOAT);
    m_vao->enable(0);
    
    m_vao->unbind();
}

void CharacterDrawable::draw()
{
    m_vao->bind();
    m_vao->drawArrays(GL_TRIANGLE_FAN, 0, 4);
    m_vao->unbind();
}
