#include "particledrawable.h"

#include <cassert>

#include <glow/logging.h>
#include <glow/VertexArrayObject.h>
#include <glow/VertexAttributeBinding.h>
#include <glow/Buffer.h>
#include <glow/Program.h>
#include <glowutils/File.h>
#include <glowutils/Camera.h>

#include <foundation/PxVec3.h>
#include <particles/PxParticleReadData.h>

#include <glm/glm.hpp>

std::list<ParticleDrawable*> ParticleDrawable::s_instances;

ParticleDrawable::ParticleDrawable(unsigned int maxParticleCount)
: m_maxParticleCount(maxParticleCount)
, m_currentNumParticles(0)
, m_vertices(new glow::Vec3Array)
, m_needBufferUpdate(true)
, m_vao(nullptr)
, m_vbo(nullptr)
, m_program(nullptr)
{
    s_instances.push_back(this);
    m_vertices->resize(m_maxParticleCount);
}

ParticleDrawable::~ParticleDrawable()
{
    s_instances.remove(this);
}

void ParticleDrawable::drawParticles(const glowutils::Camera & camera)
{
    for (auto & instance : s_instances)
        instance->draw(camera);
}

void ParticleDrawable::draw(const glowutils::Camera & camera)
{
    if (!m_vao)
        initialize();
    if (m_needBufferUpdate)
        updateBuffers();

    m_program->use();
    m_program->setUniform("viewProjection", camera.viewProjection());

    m_vao->bind();

    glEnable(GL_PROGRAM_POINT_SIZE);
    m_vao->drawArrays(GL_POINTS, 0, m_currentNumParticles);

    m_vao->unbind();

    m_program->release();
}

void ParticleDrawable::initialize()
{
    assert(m_vertices);

    m_vao = new glow::VertexArrayObject;

    m_vao->bind();
    
    m_vbo = new glow::Buffer(GL_ARRAY_BUFFER);
    m_vbo->setData(*m_vertices, GL_DYNAMIC_DRAW);

    glow::VertexAttributeBinding * vertexBinding = m_vao->binding(0);
    vertexBinding->setAttribute(0);
    vertexBinding->setBuffer(m_vbo, 0, sizeof(glm::vec3));
    vertexBinding->setFormat(3, GL_FLOAT, GL_FALSE, 0);
    m_vao->enable(0);

    m_vao->unbind();

    glow::ref_ptr<glow::Shader> particleWaterVert = glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/particle_water.vert");
    glow::ref_ptr<glow::Shader> particleWaterFrag = glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/particle_water.frag");

    m_program = new glow::Program();
    m_program->attach(particleWaterFrag, particleWaterVert);
}

void ParticleDrawable::updateBuffers()
{
    m_vbo->setData(*m_vertices, GL_DYNAMIC_DRAW);

    m_needBufferUpdate = false;
}

void ParticleDrawable::addParticles(unsigned int numParticles, const physx::PxVec3 * particlePositionBuffer)
{
    assert(numParticles <= m_maxParticleCount);

    m_currentNumParticles += numParticles;
    if (m_currentNumParticles > m_maxParticleCount)
        m_currentNumParticles = m_maxParticleCount;
    
    for (unsigned i = 0; i < numParticles; ++i) {
        const physx::PxVec3 & vertex = particlePositionBuffer[i];
        m_vertices->at(i) = glm::vec3(vertex.x, vertex.y, vertex.z);
    }

    m_needBufferUpdate = true;
}

void ParticleDrawable::updateParticles(const physx::PxParticleReadData * readData)
{
    unsigned numParticles = readData->numValidParticles;

    if (numParticles == 0)
        return;

    // assert(numParticles <= m_maxParticleCount);
    if (numParticles > m_maxParticleCount) {
        glow::warning("ParticleDrawable::updateParticles: recieving more valid new particles than expected (%;)", numParticles);
        numParticles = m_maxParticleCount;
    }

    auto pxPositionIt = readData->positionBuffer;

    for (unsigned i = 0; i < numParticles; ++i, ++pxPositionIt) {
        assert(pxPositionIt.ptr());
        const physx::PxVec3 & vertex = *pxPositionIt.ptr();
        m_vertices->at(i) = glm::vec3(vertex.x, vertex.y, vertex.z);
    }

    m_needBufferUpdate = true;
}
