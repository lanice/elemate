#include "drawable.h"

#include <cassert>

#include <glow/VertexArrayObject.h>
#include <glow/Buffer.h>


Drawable::Drawable()
: m_vao(nullptr)
, m_vbo(nullptr)
{
}

Drawable::~Drawable()
{
}

void Drawable::initialize()
{
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

void Drawable::setBoudingBox(const glowutils::AxisAlignedBoundingBox & bbox)
{
    m_bbox = bbox;
}
