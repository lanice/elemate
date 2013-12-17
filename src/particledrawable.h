#pragma once

#include <vector>

#include <osg/Drawable>

namespace physx {
    class PxVec3;
    class PxParticleReadData;
}

class ParticleDrawable : public osg::Drawable
{
public:
    /** creates a new drawable with fixed maximum number of particles */
    ParticleDrawable(unsigned int maxParticleCount);

    /** adds numParticles particles from particlePositionBuffer to the internal buffer */
    void addParticles(unsigned int numParticles, const physx::PxVec3 * particlePositionBuffer);

    /** fetches the number of valid particles and the particle positions from readData and updates interal buffers */
    void updateParticles(const physx::PxParticleReadData * readData);

    /** drawing implementation called during rendering */
    virtual void drawImplementation(osg::RenderInfo& renderInfo) const override;

    /** @return bouding box of currently stored particles */
    virtual osg::BoundingBox computeBound() const override;

protected:
    const unsigned int m_maxParticleCount;
    unsigned int m_currentNumParticles;

    std::vector<osg::Vec3f> m_vertices;

    mutable bool m_needGLUpdate;
    mutable GLuint m_vbo;
    void updateGLObjects(osg::RenderInfo& renderInfo) const;

    virtual void releaseGLObjects(osg::State* state) const override;


    ParticleDrawable() = delete;
    virtual inline osg::Object * clone(const osg::CopyOp& /*copyop*/) const override {
        return nullptr;
    }
    virtual inline osg::Object * cloneType() const override {
        return nullptr;
    }

};
