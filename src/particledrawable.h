#pragma once

#include <vector>
#include <cassert>

#include <osg/Drawable>

namespace physx {
    class PxVec3;
    class PxParticleReadData;
}

class ParticleDrawable : public osg::Drawable
{
public:
    ParticleDrawable(unsigned int maxParticleCount);

    void addParticles(unsigned int numParticles, const physx::PxVec3 * particlePositionBuffer);

    void updateParticles(const physx::PxParticleReadData * readData);

    virtual void drawImplementation(osg::RenderInfo& renderInfo) const override;

    virtual osg::BoundingBox computeBound() const override;

protected:
    const unsigned int m_maxParticleCount;
    unsigned int m_currentNumParticles;

    std::vector<osg::Vec3> m_vertices;

    mutable bool m_needGLUpdate;
    void updateGLObjects(osg::RenderInfo& renderInfo) const;

    virtual inline osg::Object * clone(const osg::CopyOp& /*copyop*/) const override {
        return nullptr;
    }
    virtual inline osg::Object * cloneType() const override {
        return nullptr;
    }

};