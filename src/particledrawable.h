#pragma once

#include <list>
#include <memory>

#include <glow/ref_ptr.h>
#include <glow/Array.h>

#include <glm/glm.hpp>

namespace glow {
    class VertexArrayObject;
    class Buffer;
    class Program;
}
namespace glowutils {
    class Camera;
}

namespace physx {
    class PxVec3;
    class PxParticleReadData;
}

class ParticleDrawable
{
public:
    /** creates a new drawable with fixed maximum number of particles */
    ParticleDrawable(unsigned int maxParticleCount);

    virtual ~ParticleDrawable();

    void initialize();
    
    /** adds numParticles particles from particlePositionBuffer to the internal buffer */
    void addParticles(unsigned int numParticles, const physx::PxVec3 * particlePositionBuffer);

    /** fetches the number of valid particles and the particle positions from readData and updates interal buffers */
    void updateParticles(const physx::PxParticleReadData * readData);

    /** draw all instances of this drawable */
    static void drawParticles(const glowutils::Camera & camera);

    /** drawing implementation called during rendering */
    virtual void draw(const glowutils::Camera & camera);

protected:
    static std::list<ParticleDrawable*> s_instances;

    const unsigned int m_maxParticleCount;
    unsigned int m_currentNumParticles;

    bool m_needBufferUpdate;
    void updateBuffers();

    glow::ref_ptr<glow::VertexArrayObject> m_vao;
    glow::ref_ptr<glow::Buffer> m_vbo;
    std::shared_ptr<glow::Vec3Array> m_vertices;
    glow::ref_ptr<glow::Program> m_program;

private:
    ParticleDrawable() = delete;
    void operator=(ParticleDrawable&) = delete;
};
