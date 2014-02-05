#include "particledrawable.h"

#include <cassert>

#include <glow/logging.h>
#include <glow/VertexArrayObject.h>
#include <glow/VertexAttributeBinding.h>
#include <glow/Buffer.h>
#include <glow/Program.h>
#include <glowutils/File.h>
#include "cameraex.h"

#include "pxcompilerfix.h"
#include <foundation/PxVec3.h>
#include <particles/PxParticleReadData.h>

#include <glm/glm.hpp>

#include "world.h"

using namespace physx;

std::list<ParticleDrawable*> ParticleDrawable::s_instances;

uint8_t ParticleDrawable::elementIndex(const std::string & elementName)
{
    if (elementName == "water")
        return 1;
    if (elementName == "lava")
        return 2;
    if (elementName == "sand")
        return 3;
    if (elementName == "bedrock")
        return 4;
    assert(false);
    return 0;   // mean unset
}

ParticleDrawable::ParticleDrawable(const std::string & elementName, unsigned int maxParticleCount)
: m_elementName(elementName)
, m_elementIndex(elementIndex(elementName))
, m_maxParticleCount(maxParticleCount)
, m_currentNumParticles(0)
, m_particleSize(1.0f)
, m_needBufferUpdate(true)
, m_vao(nullptr)
, m_vbo(nullptr)
, m_vertices(std::make_shared<glow::Vec3Array>())
, m_program(nullptr)
{
    s_instances.push_back(this);
    m_vertices->resize(m_maxParticleCount);
}

ParticleDrawable::~ParticleDrawable()
{
    s_instances.remove(this);
}

void ParticleDrawable::setParticleSize(float particleSize)
{
    assert(particleSize > 0);
    m_particleSize = particleSize;
    if (m_program)
        m_program->setUniform("particleSize", m_particleSize);
}

void ParticleDrawable::drawParticles(const CameraEx & camera)
{
    for (auto & instance : s_instances)
        instance->draw(camera);
}

void ParticleDrawable::draw(const CameraEx & camera)
{
    if (!m_vao)
        initialize();
    if (m_needBufferUpdate)
        updateBuffers();

    m_program->use();
    m_program->setUniform("viewProjection", camera.viewProjectionEx());
    m_program->setUniform("projection", camera.projectionEx());
    m_program->setUniform("view", camera.view());
    glm::vec3 viewDir = camera.center() - camera.eye();
    glm::vec3 lookAtRight = glm::normalize(glm::cross(viewDir, camera.up()));
    glm::vec3 lookAtUp = glm::normalize(glm::cross(lookAtRight, viewDir));
    m_program->setUniform("lookAtUp", lookAtUp);
    m_program->setUniform("lookAtRight", lookAtRight);
    m_program->setUniform("lookAtFront", glm::normalize(viewDir));
    m_program->setUniform("znear", camera.zNearEx());
    m_program->setUniform("zfar", camera.zFarEx());

    m_vao->bind();

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

    m_program = new glow::Program();
    m_program->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/particle/particle.vert"),
        glowutils::createShaderFromFile(GL_GEOMETRY_SHADER, "shader/particle/particle.geo"),
        World::instance()->sharedShader(GL_FRAGMENT_SHADER, "shader/depth_util.frag"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/particle/particle.frag"));

    m_program->setUniform("particleSize", m_particleSize);
    m_program->setUniform("elementIndex", m_elementIndex);
}

void ParticleDrawable::updateBuffers()
{
    m_vbo->setData(*m_vertices, GL_DYNAMIC_DRAW);

    m_needBufferUpdate = false;
}

void ParticleDrawable::updateParticles(const PxParticleReadData * readData)
{
    unsigned numParticles = readData->nbValidParticles;

    if (numParticles == 0)
        return;

    assert(numParticles <= m_maxParticleCount);
    if (numParticles > m_maxParticleCount) {
        glow::warning("ParticleDrawable::updateParticles: recieving more valid new particles than expected (%;)", numParticles);
        numParticles = m_maxParticleCount;
    }

    PxStrideIterator<const PxVec3> pxPositionIt = readData->positionBuffer;
    PxStrideIterator<const PxParticleFlags> pxFlagIt = readData->flagsBuffer;
    unsigned int nextPointIndex = 0;

    for (unsigned i = 0; i < readData->validParticleRange; ++i, ++pxPositionIt, ++pxFlagIt) {
        assert(pxPositionIt.ptr());
        if (*pxFlagIt & PxParticleFlag::eVALID) {
            const physx::PxVec3 & vertex = *pxPositionIt;
            m_vertices->at(nextPointIndex++) = glm::vec3(vertex.x, vertex.y, vertex.z);
        }
    }

    m_currentNumParticles = nextPointIndex;

    m_needBufferUpdate = true;
}