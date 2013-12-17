#pragma once

#define DISALLOW_COPY_AND_ASSIGN(TypeName)  \
    TypeName(const TypeName&);              \
    void operator=(const TypeName&);

#include "PxPhysicsAPI.h"
#include <list>
#include <osg/GraphicsContext> // ref_ptr

namespace osg {
    class MatrixTransform;
    class Group;
}

typedef long double t_longf;

class ParticleEmitter
{
public:
    ParticleEmitter(osg::ref_ptr<osg::Group> parent, const physx::PxVec3& position);
    ParticleEmitter(osg::ref_ptr<osg::Group> parent);
    ~ParticleEmitter();

    void initializeParticleSystem();
    void update(t_longf elapsed_Time);

    void startEmit();
    void stopEmit();

    void createParticles(int number_of_particles);

protected:
    static const physx::PxU32	kMaxParticleCount = 100;
    static const physx::PxU32   kDefaultEmittedParticles = 1;

    osg::ref_ptr<osg::Group> m_parent;
    physx::PxVec3            m_position;
    bool                     m_emitting;
    int                      m_particles_per_second;


    physx::PxParticleSystem*            m_particle_system; // or fluid?
    std::list<osg::MatrixTransform*>    m_particle_objects;

    void createParticleObjects();

private:
    DISALLOW_COPY_AND_ASSIGN(ParticleEmitter);
};