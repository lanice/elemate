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
namespace physx {
    class PxVec3;
    class PxParticleReadData;
}
class CameraEx;

class ParticleDrawable
{
public:
    /** creates a new drawable with fixed maximum number of particles */
    ParticleDrawable(const std::string & elementName, unsigned int maxParticleCount);

    virtual ~ParticleDrawable();

    void initialize();

    /** fetches the number of valid particles and the particle positions from readData and updates interal buffers */
    void updateParticles(const physx::PxParticleReadData * readData);

    void setParticleSize(float particleSize);

    /** draw all instances of this drawable */
    static void drawParticles(const CameraEx & camera);

    /** drawing implementation called during rendering */
    virtual void draw(const CameraEx & camera);

protected:
    static std::list<ParticleDrawable*> s_instances;

    const std::string m_elementName;
    const uint8_t m_elementIndex;

    static uint8_t elementIndex(const std::string & elementName);

    const unsigned int m_maxParticleCount;
    unsigned int m_currentNumParticles;

    float m_particleSize;

    bool m_needBufferUpdate;
    void updateBuffers();

    glow::ref_ptr<glow::VertexArrayObject> m_vao;
    glow::ref_ptr<glow::Buffer> m_vbo;
    std::shared_ptr<glow::Vec3Array> m_vertices;
    glow::ref_ptr<glow::Program> m_program;

public:
    ParticleDrawable() = delete;
    ParticleDrawable(ParticleDrawable&) = delete;
    void operator=(ParticleDrawable&) = delete;
};
