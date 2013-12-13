#include "particledrawable.h"

#include <iostream>

#include <foundation/PxVec3.h>
#include <particles/PxParticleReadData.h>

ParticleDrawable::ParticleDrawable(unsigned int maxParticleCount)
: m_maxParticleCount(maxParticleCount)
, m_currentNumParticles(0)
, m_vertices(std::vector<osg::Vec3>(maxParticleCount, osg::Vec3(0, 0, 0)))
{
    setUseDisplayList(false);
    //setUseVertexBufferObjects(true);
}

void ParticleDrawable::drawImplementation(osg::RenderInfo& renderInfo) const
{
    std::cout << "draw" << std::endl;
    updateGLObjects(renderInfo);
    return;
}

void ParticleDrawable::updateGLObjects(osg::RenderInfo& info) const
{
    if (!m_needGLUpdate)
        return;

    m_needGLUpdate = false;
    std::cout << "updating gl objects" << std::endl;
}

void ParticleDrawable::addParticles(unsigned int numParticles, const physx::PxVec3 * particlePositionBuffer)
{
    assert(numParticles <= m_maxParticleCount);

    m_currentNumParticles += numParticles;
    if (m_currentNumParticles > m_maxParticleCount)
        m_currentNumParticles = m_maxParticleCount;
    
    for (unsigned i = 0; i < numParticles; ++i) {
        const physx::PxVec3 & vertex = particlePositionBuffer[i];
        m_vertices[i] = osg::Vec3(vertex.x, vertex.y, vertex.z);
    }

    m_needGLUpdate = true;
    std::cout << "add "<< numParticles << " particles" << std::endl;
}

void ParticleDrawable::updateParticles(const physx::PxParticleReadData * readData)
{
    auto pxPositionIt = readData->positionBuffer;

    releaseGLObjects();

    assert(readData->numValidParticles <= m_maxParticleCount);

    for (unsigned i = 0; i < readData->numValidParticles; ++i, ++pxPositionIt) {
        assert(pxPositionIt.ptr());
        const physx::PxVec3 & vertex = *pxPositionIt.ptr();
        m_vertices[i] = osg::Vec3(vertex.x, vertex.y, vertex.z);
    }
}

osg::BoundingBox ParticleDrawable::computeBound() const
{
    osg::BoundingBox bb;

    for (const osg::Vec3 & vertex : m_vertices)
        bb.expandBy(vertex);

    return bb;
}
