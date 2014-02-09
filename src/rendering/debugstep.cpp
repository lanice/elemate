#include "debugstep.h"

#include <glow/VertexArrayObject.h>
#include <glow/VertexAttributeBinding.h>
#include <glow/Buffer.h>
#include <glow/Program.h>
#include <glowutils/File.h>

#include "drawable.h"
#include "world.h"
#include "particles/particlecollision.h"
#include "utils/cameraex.h"

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

    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);

    m_wireframeBoxProgram->use();
    m_wireframeBoxProgram->setUniform("color", glm::vec4(0, 1, 0, 1));
    for (const Drawable * drawable : Drawable::instances()) {
        const glowutils::AxisAlignedBoundingBox & bbox = drawable->boundingBox();

        if (bbox == glowutils::AxisAlignedBoundingBox())    // don't draw zero-initialized boxes
            continue;

        m_wireframeBoxProgram->setUniform("MVP", camera.viewProjectionEx() * drawable->transform());

        m_vbo->setData(glow::Vec3Array({ bbox.llf(), bbox.urb() }), GL_DYNAMIC_DRAW);

        m_vao->drawArrays(GL_LINES, 0, 2);
    }
    m_wireframeBoxProgram->release();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    m_solidBoxProgram->setUniform("MVP", camera.viewProjectionEx());
    m_solidBoxProgram->setUniform("color", glm::vec4(1, 0, 0, 0.28));
    for (const ParticleCollision::IntersectionBox & ibox : ParticleCollision::debug_intersectionBoxes) {
        m_vbo->setData(glow::Vec3Array({ibox.llf, ibox.urb}), GL_DYNAMIC_DRAW);

        m_vao->drawArrays(GL_LINES, 0, 2);
    }
    glDisable(GL_BLEND);
    m_solidBoxProgram->release();

    m_vao->unbind();
    m_vbo->unbind();

    glDisable(GL_CULL_FACE);
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

    m_wireframeBoxProgram = new glow::Program();
    m_wireframeBoxProgram->attach(
        World::instance()->sharedShader(GL_VERTEX_SHADER, "shader/debug/box.vert"),
        glowutils::createShaderFromFile(GL_GEOMETRY_SHADER, "shader/debug/wireframeBox.geo"),
        World::instance()->sharedShader(GL_FRAGMENT_SHADER, "shader/debug/box.frag"));

    m_solidBoxProgram = new glow::Program();
    m_solidBoxProgram->attach(
        World::instance()->sharedShader(GL_VERTEX_SHADER, "shader/debug/box.vert"),
        glowutils::createShaderFromFile(GL_GEOMETRY_SHADER, "shader/debug/solidBox.geo"),
        World::instance()->sharedShader(GL_FRAGMENT_SHADER, "shader/debug/box.frag"));
}
