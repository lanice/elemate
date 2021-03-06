#include "particledrawable.h"

#include <cassert>

#include <glow/logging.h>
#include <glow/VertexArrayObject.h>
#include <glow/VertexAttributeBinding.h>
#include <glow/Buffer.h>
#include <glow/Program.h>
#include <glowutils/global.h>
#include "utils/cameraex.h"

#include "utils/pxcompilerfix.h"
#include <foundation/PxVec3.h>
#include <particles/PxParticleReadData.h>

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
    if (elementName == "steam")
        return 5;
    assert(false);
    return 0;   // mean unset
}

ParticleDrawable::ParticleDrawable(const std::string & elementName, unsigned int maxParticleCount, bool isDown)
: Drawable()
, isDown(isDown)
, m_elementName(elementName)
, m_elementIndex(elementIndex(elementName))
, m_maxParticleCount(maxParticleCount)
, m_currentNumParticles(0)
, m_particleSize(1.0f)
, m_needBufferUpdate(true)
, m_program(nullptr)
{
    s_instances.push_back(this);
    m_vertices.resize(m_maxParticleCount);
}

void ParticleDrawable::setElement(const std::string & elementName)
{
    m_elementName = elementName;
    m_elementIndex = elementIndex(elementName);
    if (m_program)
        m_program->setUniform("elementIndex", m_elementIndex);
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

void ParticleDrawable::drawImplementation(const CameraEx & camera)
{
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

    m_vao->drawArrays(GL_POINTS, 0, m_currentNumParticles);

    m_program->release();
}

void ParticleDrawable::initialize()
{
    m_vao = new glow::VertexArrayObject;

    m_vao->bind();
    
    m_vbo = new glow::Buffer(GL_ARRAY_BUFFER);
    m_vbo->setData(m_maxParticleCount * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

    m_needBufferUpdate = true;

    glow::VertexAttributeBinding * vertexBinding = m_vao->binding(0);
    vertexBinding->setAttribute(0);
    vertexBinding->setBuffer(m_vbo, 0, sizeof(glm::vec3));
    vertexBinding->setFormat(3, GL_FLOAT, GL_FALSE, 0);
    m_vao->enable(0);

    m_vao->unbind();

    m_program = new glow::Program();
    m_program->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/particles/particle.vert"),
        glowutils::createShaderFromFile(GL_GEOMETRY_SHADER, "shader/particles/particle.geo"),
        World::instance()->sharedShader(GL_FRAGMENT_SHADER, "shader/utils/depth_util.frag"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/particles/particle.frag"));

    m_program->setUniform("particleSize", m_particleSize);
    m_program->setUniform("elementIndex", m_elementIndex);
}

void ParticleDrawable::updateBuffers()
{
    assert(m_vertices.size() <= m_maxParticleCount * sizeof(glm::vec3));
    
    glm::vec3 * particleGpuDest = reinterpret_cast<glm::vec3*>(
        m_vbo->mapRange(0, m_vertices.size() * sizeof(glm::vec3), GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT));
    assert(particleGpuDest);
    memcpy(particleGpuDest, static_cast<void*>(m_vertices.data()), m_vertices.size() * sizeof(glm::vec3));
    m_vbo->unmap();

    m_needBufferUpdate = false;
}

void ParticleDrawable::updateParticles(const PxParticleReadData * readData)
{
    assert(readData);
    unsigned numParticles = readData->nbValidParticles;

    m_bbox = glowutils::AxisAlignedBoundingBox();

    if (numParticles == 0 && m_currentNumParticles == 0)
        return;

    assert(numParticles <= m_maxParticleCount);
    if (numParticles > m_maxParticleCount) {
        glow::warning("ParticleDrawable::updateParticles: receiving more valid new particles than expected (%;)", numParticles);
        numParticles = m_maxParticleCount;
    }

    PxStrideIterator<const PxVec3> pxPositionIt = readData->positionBuffer;
    PxStrideIterator<const PxParticleFlags> pxFlagIt = readData->flagsBuffer;
    unsigned int nextPointIndex = 0;

    for (unsigned i = 0; i < readData->validParticleRange; ++i, ++pxPositionIt, ++pxFlagIt) {
        assert(pxPositionIt.ptr());
        if (*pxFlagIt & PxParticleFlag::eVALID) {
            const glm::vec3 & vertex = reinterpret_cast<const glm::vec3&>(*pxPositionIt.ptr());
            m_vertices.at(nextPointIndex) = vertex;
            m_bbox.extend(vertex);
            ++nextPointIndex;
        }
    }

    assert((m_bbox.llf().x != std::numeric_limits<float>::max()) == (numParticles > 0));

    m_currentNumParticles = nextPointIndex;

    m_needBufferUpdate = true;
}
