#include "debugstep.h"

#include <glow/VertexArrayObject.h>
#include <glow/VertexAttributeBinding.h>
#include <glow/Buffer.h>
#include <glow/Program.h>
#include <glowutils/File.h>

#include "drawable.h"
#include "cameraex.h"

bool operator==(const glowutils::AxisAlignedBoundingBox & lhs, const glowutils::AxisAlignedBoundingBox & rhs)
{
    if (&lhs == &rhs)
        return true;

    if (lhs.llf() != rhs.llf())
        return false;

    return lhs.urb() == rhs.urb();
}

void DebugStep::draw(const CameraEx & camera)
{
    if (!m_vao)
        initialize();

    m_bboxProgram->use();

    for (const Drawable * drawable : Drawable::instances()) {
        const glowutils::AxisAlignedBoundingBox & bbox = drawable->boundingBox();

        if (bbox == glowutils::AxisAlignedBoundingBox())    // don't draw zero-initialized boxes
            continue;

        m_bboxProgram->setUniform("MVP", camera.viewProjectionEx() * drawable->transform());

        m_vbo->setData(glow::Vec3Array({ bbox.llf(), bbox.urb() }), GL_DYNAMIC_DRAW);

        m_vao->drawArrays(GL_LINES, 0, 2);
    }

    m_bboxProgram->release();
}

void DebugStep::resize(int /*width*/, int /*height*/)
{
}

void DebugStep::initialize()
{
    m_vao = new glow::VertexArrayObject;

    m_vao->bind();

    m_vbo = new glow::Buffer(GL_ARRAY_BUFFER);

    glow::VertexAttributeBinding * vertexBinding = m_vao->binding(0);
    vertexBinding->setAttribute(0);
    vertexBinding->setBuffer(m_vbo, 0, sizeof(glm::vec3));
    vertexBinding->setFormat(3, GL_FLOAT, GL_FALSE, 0);
    m_vao->enable(0);

    m_vao->unbind();

    m_bboxProgram = new glow::Program();
    m_bboxProgram->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/boundingbox.vert"),
        glowutils::createShaderFromFile(GL_GEOMETRY_SHADER, "shader/boundingbox.geo"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/boundingbox.frag"));
}
