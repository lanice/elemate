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
    /** creates a new drawable with fixed maximum number of particles.
        @param elementName element used in the flush step */
    ParticleDrawable(const std::string & elementName, unsigned int maxParticleCount, bool isDown);
    virtual ~ParticleDrawable() override;

    /** set used element for the particle drawable. */
    void setElement(const std::string & elementName);

    /** Specify in the groups constructor if it is emitting or down. Used to emit a dynamic_cast on subclasses */
    bool isDown;

    /** fetches the number of valid particles and the particle positions from readData and updates the vertex buffers data */
    void updateParticles(const physx::PxParticleReadData * readData);

    /** set the particles size used for shading */
    void setParticleSize(float particleSize);

    /** draw all instances of this drawable */
    static void drawParticles(const CameraEx & camera);

protected:
    /** The ParticleGroup may directly set the bounding box of the drawable to omit to frequent reading of PhysX data structures. */
    friend class ParticleGroup;

    /** list of all particle drawables, allowing to draw all instances in the drawParticles call. */
    static std::list<ParticleDrawable*> s_instances;

    /** initialize the vertex buffer, array object and program  */
    virtual void initialize() override;

    virtual void drawImplementation(const CameraEx & camera) override;

    std::string m_elementName;
    uint8_t m_elementIndex;

    /** get an id for the element, to distinguish rendering elements in the flush step */
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
