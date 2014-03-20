#pragma once

#include "drawable.h"

#include <list>
#include <memory>
#include <vector>

#include <glow/ref_ptr.h>

#include <glm/glm.hpp>

namespace glow {
    class Program;
}
namespace physx {
    class PxParticleReadData;
}
class CameraEx;

class ParticleDrawable : public Drawable
{
public:
    /** creates a new drawable with fixed maximum number of particles */
    ParticleDrawable(const std::string & elementName, unsigned int maxParticleCount, bool isDown);
    void setElement(const std::string & elementName);
    bool isDown;

    virtual ~ParticleDrawable();

    void initialize();

    /** fetches the number of valid particles and the particle positions from readData and updates the vertex buffers data */
    void updateParticles(const physx::PxParticleReadData * readData);

    void setParticleSize(float particleSize);

    /** draw all instances of this drawable */
    static void drawParticles(const CameraEx & camera);

    friend class ParticleGroup;

protected:
    static std::list<ParticleDrawable*> s_instances;

    virtual void drawImplementation(const CameraEx & camera) override;

    std::string m_elementName;
    uint8_t m_elementIndex;

    static uint8_t elementIndex(const std::string & elementName);

    const unsigned int m_maxParticleCount;
    unsigned int m_currentNumParticles;

    float m_particleSize;

    bool m_needBufferUpdate;
    void updateBuffers();

    std::vector<glm::vec3> m_vertices;
    glow::ref_ptr<glow::Program> m_program;

public:
    ParticleDrawable() = delete;
    ParticleDrawable(ParticleDrawable&) = delete;
    void operator=(ParticleDrawable&) = delete;
};
