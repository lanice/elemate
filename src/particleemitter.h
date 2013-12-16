#pragma once

#define DISALLOW_COPY_AND_ASSIGN(TypeName)  \
    TypeName(const TypeName&);              \
    void operator=(const TypeName&);

#include "PxPhysicsAPI.h"
#include <list>

namespace osg{
    class MatrixTransform;
}

class ParticleEmitter
{
public:
    ParticleEmitter();
    ~ParticleEmitter();

    void initializeParticleSystem();
    void update();

    void startEmit();
    void stopEmit();

protected:
    physx::PxParticleSystem*            m_particle_system; // or fluid?
    std::list<osg::MatrixTransform*>    m_particle_objects;
private:
    DISALLOW_COPY_AND_ASSIGN(ParticleEmitter);
};