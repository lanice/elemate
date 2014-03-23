#include "drawable.h"

#include <cassert>

#include <glow/VertexArrayObject.h>
#include <glow/Buffer.h>

#include <glm/glm.hpp>

std::set<Drawable*> Drawable::s_drawableInstances;

Drawable::Drawable()
: m_vao(nullptr)
, m_vbo(nullptr)
{
    s_drawableInstances.insert(this);
}

Drawable::~Drawable()
{
    s_drawableInstances.erase(this);
}

void Drawable::draw(const CameraEx & camera)
{
    if (!m_vao)
        initialize();

    assert(m_vao);
    assert(m_vbo);

    m_vao->bind();

    drawImplementation(camera);

    m_vao->unbind();
}

const glowutils::AxisAlignedBoundingBox & Drawable::boundingBox() const
{
    return m_bbox;
}

const glm::mat4 & Drawable::transform() const
{
    static const glm::mat4 identity;
    return identity;
}
