#include "particleemitter.h"

#include "particledrawable.h"

#include <osg/Point>
#include <osg/ShapeDrawable>
#include <osgViewer/View>
#include <osg/MatrixTransform>

#include <cassert>

ParticleEmitter::ParticleEmitter(osg::ref_ptr<osg::Group> parent, const physx::PxVec3& position):
    m_parent(parent),
    m_position(position),
    m_emitting(false),
    akkumulator(1.0f/60.0f),
    youngest_particle_index(0),
    m_particles_per_second(kDefaultEmittedParticles)
{
}
ParticleEmitter::ParticleEmitter(osg::ref_ptr<osg::Group> parent):
    ParticleEmitter(parent,physx::PxVec3(0,0,0))
{
}

ParticleEmitter::~ParticleEmitter(){
    m_particle_system->releaseParticles();
}

void ParticleEmitter::initializeParticleSystem(){
    m_particle_group = m_parent;

    m_particle_system = PxGetPhysics().createParticleFluid(kMaxParticleCount, false);
    assert(m_particle_system);
    m_particle_system->setRestParticleDistance(0.3F);

    physx::PxScene* scene_buffer = static_cast<physx::PxScene*>(malloc(sizeof(physx::PxScene)));
    PxGetPhysics().getScenes(&scene_buffer, 1);
    scene_buffer->addActor(*m_particle_system);

    osg::ref_ptr<ParticleDrawable> waterDrawable = new ParticleDrawable(kMaxParticleCount);
    osg::ref_ptr<osg::Geode> waterGeode = new osg::Geode;
    waterGeode->addDrawable(waterDrawable);

    m_particle_drawable = waterDrawable.get();

    assert(m_particle_group.valid());
    m_particle_group->addChild(waterGeode.get());

    m_particle_group->getOrCreateStateSet()->setAttribute(new osg::Point(10.0f), osg::StateAttribute::ON);

}

void ParticleEmitter::update(t_longf elapsed_Time){
    if (akkumulator > 0 && m_emitting){
        akkumulator -= elapsed_Time;
        return;
    } else {
        akkumulator = 1.0f / 60.0f;
        createParticles(m_particles_per_second);
    }
    
    physx::PxParticleReadData * read_data = m_particle_system->lockParticleReadData();
    assert(read_data);

    m_particle_drawable->updateParticles(read_data);

    read_data->unlock();
}

void ParticleEmitter::startEmit(){
    m_emitting = true;
}

void ParticleEmitter::createParticles(int number_of_particles){
    assert(m_particle_group.valid());

    if (number_of_particles > kMaxParticleCount)
        number_of_particles = kMaxParticleCount;

    physx::PxParticleCreationData particleCreationData;
    particleCreationData.numParticles = number_of_particles;
    physx::PxU32    m_particle_index_buffer[kMaxParticleCount];
    physx::PxVec3   m_particle_position_buffer[kMaxParticleCount];
    physx::PxVec3   m_particle_velocity_buffer[kMaxParticleCount];

    for (size_t i = 0; i < number_of_particles; i++)
    {
        m_particle_index_buffer[i] = (youngest_particle_index + i) % kMaxParticleCount;
        m_particle_position_buffer[i] = m_position;
        m_particle_velocity_buffer[i] = physx::PxVec3(0.0F + (rand() % 4 - 2), 0.0F + (rand() % 4 - 2), 0.0F + (rand() % 4 - 2));
    }
    youngest_particle_index = (youngest_particle_index + number_of_particles) % kMaxParticleCount;

    particleCreationData.indexBuffer = physx::PxStrideIterator<const physx::PxU32>(m_particle_index_buffer);
    particleCreationData.positionBuffer = physx::PxStrideIterator<const physx::PxVec3>(m_particle_position_buffer);
    particleCreationData.velocityBuffer = physx::PxStrideIterator<const physx::PxVec3>(m_particle_velocity_buffer);

    assert(m_particle_system->createParticles(particleCreationData));

    osg::ref_ptr<ParticleDrawable> waterDrawable = m_particle_drawable.get();
    waterDrawable->addParticles(number_of_particles, m_particle_position_buffer);
}

void ParticleEmitter::stopEmit(){
    m_emitting = false;
}

