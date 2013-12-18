#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace physx {
    class PxVec3;
    class PxParticleReadData;
}

class ParticleDrawable
{
public:
    /** creates a new drawable with fixed maximum number of particles */
    ParticleDrawable(unsigned int maxParticleCount);

    void initialize();
    
    /** adds numParticles particles from particlePositionBuffer to the internal buffer */
    void addParticles(unsigned int numParticles, const physx::PxVec3 * particlePositionBuffer);

    /** fetches the number of valid particles and the particle positions from readData and updates interal buffers */
    void updateParticles(const physx::PxParticleReadData * readData);

    /** drawing implementation called during rendering */
    virtual void draw();

protected:
    const unsigned int m_maxParticleCount;
    unsigned int m_currentNumParticles;

    std::vector<glm::vec3> m_vertices;

    mutable bool m_needGLUpdate;
    //mutable GLuint m_vbo;

private:
    ParticleDrawable() = delete;
    void operator=(ParticleDrawable&) = delete;
};
