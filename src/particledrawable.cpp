#include "particledrawable.h"

#include <iostream>
#include <cassert>

#include <foundation/PxVec3.h>
#include <particles/PxParticleReadData.h>

#include <glm/glm.hpp>

ParticleDrawable::ParticleDrawable(unsigned int maxParticleCount)
: m_maxParticleCount(maxParticleCount)
, m_currentNumParticles(0)
, m_vertices(std::vector<glm::vec3>(maxParticleCount, glm::vec3(0, 0, 0)))
, m_needGLUpdate(true)
//, m_vbo(UINT_MAX)
{
}

void ParticleDrawable::draw()
{
    /*if (m_needGLUpdate)
        updateGLObjects(renderInfo);

    osg::State & state = *renderInfo.getState();
    Extensions & ext = *getExtensions(renderInfo.getContextID(), true);

    ext.glBindBuffer(GL_ARRAY_BUFFER_ARB, m_vbo);
    state.setVertexPointer(3, GL_FLOAT, 0, 0);

    state.glDrawArraysInstanced(GL_POINTS, 0, m_currentNumParticles, 0);

    state.disableAllVertexArrays();

    return;*/
}

void ParticleDrawable::initialize()
{

}

void ParticleDrawable::addParticles(unsigned int numParticles, const physx::PxVec3 * particlePositionBuffer)
{
    assert(numParticles <= m_maxParticleCount);

    m_currentNumParticles += numParticles;
    if (m_currentNumParticles > m_maxParticleCount)
        m_currentNumParticles = m_maxParticleCount;
    
    for (unsigned i = 0; i < numParticles; ++i) {
        const physx::PxVec3 & vertex = particlePositionBuffer[i];
        m_vertices[i] = glm::vec3(vertex.x, vertex.y, vertex.z);
    }

    m_needGLUpdate = true;
}

void ParticleDrawable::updateParticles(const physx::PxParticleReadData * readData)
{
    unsigned numParticles = readData->numValidParticles;

    if (numParticles == 0)
        return;

    // assert(numParticles <= m_maxParticleCount);
    if (numParticles > m_maxParticleCount) {
        //std::cerr << "ParticleDrawable::updateParticles: recieving more valid new particles than expected (" << numParticles << ")" << std::endl;
        numParticles = m_maxParticleCount;
    }

    auto pxPositionIt = readData->positionBuffer;

    for (unsigned i = 0; i < numParticles; ++i, ++pxPositionIt) {
        assert(pxPositionIt.ptr());
        const physx::PxVec3 & vertex = *pxPositionIt.ptr();
        m_vertices[i] = glm::vec3(vertex.x, vertex.y, vertex.z);
    }

    m_needGLUpdate = true;
}
