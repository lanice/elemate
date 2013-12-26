#include "particleemitter.h"

#include "particledrawable.h"

#include <cassert>
#include <iostream>

ParticleEmitter::ParticleEmitter(const physx::PxVec3& position):
    m_position(position),
    m_emitting(false),
    akkumulator(1.0f/60.0f),
    youngest_particle_index(0),
    m_particles_per_second(kDefaultEmittedParticles)
{
}

ParticleEmitter::~ParticleEmitter(){
    m_particleSystem->releaseParticles();
}

void ParticleEmitter::initializeParticleSystem(){
    m_particleSystem = PxGetPhysics().createParticleSystem(kMaxParticleCount, false);
    assert(m_particleSystem);

    physx::PxScene* scene_buffer = static_cast<physx::PxScene*>(malloc(sizeof(physx::PxScene)));
    PxGetPhysics().getScenes(&scene_buffer, 1);
    scene_buffer->addActor(*m_particleSystem);

    m_particleDrawable = std::make_shared<ParticleDrawable>(kMaxParticleCount);
}

void ParticleEmitter::update(t_longf elapsed_Time){
    if (akkumulator > 0 && m_emitting){
        akkumulator -= elapsed_Time;
        return;
    } else {
        akkumulator = 1.0f / 60.0f;
        createParticles(m_particles_per_second);
    }
    
    physx::PxParticleReadData * read_data = m_particleSystem->lockParticleReadData();
    assert(read_data);

    m_particleDrawable->updateParticles(read_data);

    read_data->unlock();
}

void ParticleEmitter::startEmit(){
    m_emitting = true;
}

void ParticleEmitter::createParticles(int number_of_particles){

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

    bool result = m_particleSystem->createParticles(particleCreationData);
    assert(result);

    m_particleDrawable->addParticles(number_of_particles, m_particle_position_buffer);
}

void ParticleEmitter::stopEmit(){
    m_emitting = false;
}

