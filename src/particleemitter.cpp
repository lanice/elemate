#include "particleemitter.h"

#include <cassert>

#include <glow/logging.h>

#include "PxPhysicsAPI.h"

#include "particledrawable.h"


const physx::PxU32  ParticleEmitter::kMaxParticleCount = 2000;
const physx::PxU32  ParticleEmitter::kDefaultEmittedParticles = 1;
const physx::PxReal ParticleEmitter::kDefaultInitialParticleSpeed = 0.5F;
const int           ParticleEmitter::kDefaultParticleSpreading = 50;

ParticleEmitter::ParticleEmitter(const physx::PxVec3& position)
: m_particleDrawable(nullptr)
, m_position(position)
, m_emitting(false)
, m_particles_per_second(kDefaultEmittedParticles)
, m_akkumulator(1.0f/60.0f)
, m_youngest_particle_index(0)
, m_reuses_old_particle(false)
, m_reuse_limitation(0)
{
}

ParticleEmitter::~ParticleEmitter(){
    m_particleSystem->releaseParticles();
}

void ParticleEmitter::initializeParticleSystem(EmitterDescriptionData* descriptionData){
    applyDescriptionData(descriptionData);

    assert(PxGetPhysics().getNbScenes() == 1);
    physx::PxScene* scene_buffer[1];
    PxGetPhysics().getScenes(scene_buffer, 1);
    scene_buffer[0]->addActor(*m_particleSystem);

    m_particleDrawable = std::make_shared<ParticleDrawable>(kMaxParticleCount);
}

void ParticleEmitter::applyDescriptionData(EmitterDescriptionData* descriptionData){
    m_particleSystem = PxGetPhysics().createParticleFluid(kMaxParticleCount, false);
    assert(m_particleSystem);

    m_particleSystem->setViscosity(descriptionData->viscosity);
    m_particleSystem->setStiffness(descriptionData->stiffness);
    m_particleSystem->setRestParticleDistance(descriptionData->restParticleDistance);
}

void ParticleEmitter::step(double elapsed_Time)
{
    if (m_akkumulator > 0.0f && m_emitting){
        m_akkumulator = static_cast<float>(m_akkumulator - elapsed_Time);
        return;
    } else {
        m_akkumulator = 1.0f / 60.0f;
        if (m_emitting)
            createParticles(m_particles_per_second);
    }
}

void ParticleEmitter::update()
{    
    physx::PxParticleReadData * read_data = m_particleSystem->lockParticleReadData();
    assert(read_data);

    m_particleDrawable->updateParticles(read_data);

    read_data->unlock();
}

void ParticleEmitter::startEmit(){
    m_emitting = true;
}

void ParticleEmitter::createParticles(physx::PxU32 number_of_particles){

    if (number_of_particles > kMaxParticleCount)
        number_of_particles = kMaxParticleCount;

    number_of_particles = number_of_particles + m_youngest_particle_index > kMaxParticleCount ? kMaxParticleCount - m_youngest_particle_index : number_of_particles;

    m_reuse_limitation = m_youngest_particle_index;
    
    physx::PxParticleCreationData particleCreationData;
    particleCreationData.numParticles = number_of_particles;
    physx::PxU32    m_particle_index_buffer[kMaxParticleCount];
    physx::PxVec3   m_particle_position_buffer[kMaxParticleCount];
    physx::PxVec3   m_particle_velocity_buffer[kMaxParticleCount];

    for (physx::PxU32 i = 0; i < number_of_particles; i++)
    {
        m_particle_index_buffer[i] = (m_youngest_particle_index + i) % kMaxParticleCount;
        m_particle_position_buffer[i] = m_position;
        m_particle_velocity_buffer[i] = physx::PxVec3(0.0F + (rand() % kDefaultParticleSpreading - (kDefaultParticleSpreading / 2)),
                                                      -1.0F, 
                                                      0.0F + (rand() % kDefaultParticleSpreading - (kDefaultParticleSpreading / 2)))
                                                      .getNormalized()*kDefaultInitialParticleSpeed;
    }
    m_youngest_particle_index = (m_youngest_particle_index + number_of_particles) % kMaxParticleCount;

    particleCreationData.indexBuffer = physx::PxStrideIterator<const physx::PxU32>(m_particle_index_buffer);
    particleCreationData.positionBuffer = physx::PxStrideIterator<const physx::PxVec3>(m_particle_position_buffer);
    particleCreationData.velocityBuffer = physx::PxStrideIterator<const physx::PxVec3>(m_particle_velocity_buffer);

    if (m_reuses_old_particle)
        m_particleSystem->releaseParticles(number_of_particles, particleCreationData.indexBuffer);
    m_reuses_old_particle |= m_reuse_limitation > m_youngest_particle_index;

    bool result = m_particleSystem->createParticles(particleCreationData);
    assert(result);
    if (!result) {
        glow::warning("ParticleEmitter::createParticles creation of %; physx particles failed", number_of_particles);
        return;
    }

    m_particleDrawable->addParticles(number_of_particles, m_particle_position_buffer);
}

void ParticleEmitter::stopEmit(){
    m_emitting = false;
}


void ParticleEmitter::setPosition(const physx::PxVec3& position){
    m_position = position;
}

physx::PxVec3 ParticleEmitter::position()const{
    return m_position;
}

void ParticleEmitter::setEmittingRate(const physx::PxU32& particle_per_emit){
    m_particles_per_second = particle_per_emit;
}
