#include "particlegroup.h"

#include <cassert>
#include <functional>
#include <type_traits>

#include <glow/logging.h>

#include <PxPhysics.h>
#include <PxScene.h>
#include <PxSceneLock.h>

#include "rendering/particledrawable.h"
#include "io/soundmanager.h"

using namespace physx;

ParticleGroup::ParticleGroup(
    const std::string & elementName,
    const unsigned int id,
    const bool enableGpuParticles,
    const bool isDown,
    const PxU32 maxParticleCount,
    const ImmutableParticleProperties & immutableProperties,
    const MutableParticleProperties & mutableProperties
    )
: m_particleSystem(nullptr)
, m_id(id)
, m_scene(nullptr)
, m_elementName(elementName)
, m_temperature(0.0f)
, isDown(isDown)
, m_particleDrawable(std::make_shared<ParticleDrawable>(elementName, maxParticleCount, isDown))
, m_maxParticleCount(maxParticleCount)
, m_numParticles(0)
, m_indices(new PxU32[maxParticleCount]())
, m_nextFreeIndex(0)
, m_lastFreeIndex(maxParticleCount-1)
, m_gpuParticles(enableGpuParticles)
{
    static_assert(sizeof(glm::vec3) == sizeof(physx::PxVec3), "size of physx vec3 does not match the size of glm::vec3.");

    initialize(immutableProperties, mutableProperties);
}

ParticleGroup::~ParticleGroup()
{
    PxSceneWriteLock scopedLock(* m_scene);

    m_particleSystem->releaseParticles();
    m_scene->removeActor(*m_particleSystem);
    m_particleSystem = nullptr;
    delete m_indices;
}

ParticleGroup::ParticleGroup(const ParticleGroup & lhs, unsigned int id)
: m_particleSystem(nullptr)
, m_id(id)
, m_scene(nullptr)
, m_elementName(lhs.m_elementName)
, m_temperature(lhs.m_temperature)
, isDown(true)
, m_particleDrawable(std::make_shared<ParticleDrawable>(lhs.m_elementName, lhs.m_maxParticleCount, isDown))
, m_maxParticleCount(lhs.m_maxParticleCount)
, m_numParticles(0)
, m_indices(new PxU32[lhs.m_maxParticleCount]())
, m_nextFreeIndex(0)
, m_lastFreeIndex(lhs.m_maxParticleCount - 1)
, m_gpuParticles(lhs.m_gpuParticles)
{
    initialize(lhs.m_immutableProperties, lhs.m_mutableProperties);
}

void ParticleGroup::initialize(const ImmutableParticleProperties & immutableProperties, const MutableParticleProperties & mutableProperties)
{
    m_soundChannel = SoundManager::instance()->createNewChannel("data/sounds/elements/" + m_elementName + ".wav", true, true, true);
    SoundManager::instance()->setVolume(m_soundChannel, 0.15f);

    for (PxU32 i = 0; i < m_maxParticleCount; ++i) m_indices[i] = i;

    assert(PxGetPhysics().getNbScenes() == 1);
    PxScene * pxScenePtrs[1];
    PxGetPhysics().getScenes(pxScenePtrs, 1);
    m_scene = pxScenePtrs[0];

    PxSceneWriteLock scopedLock(*m_scene);

    m_particleSystem = PxGetPhysics().createParticleFluid(m_maxParticleCount, false);
    assert(m_particleSystem);
    m_particleSystem->setParticleBaseFlag(physx::PxParticleBaseFlag::eGPU, m_gpuParticles);
    m_particleSystem->setParticleReadDataFlag(PxParticleReadDataFlag::eVELOCITY_BUFFER, true);

    m_scene->addActor(*m_particleSystem);

    setImmutableProperties(immutableProperties);
    setMutableProperties(mutableProperties);
}

const std::string & ParticleGroup::elementName() const
{
    return m_elementName;
}

uint32_t ParticleGroup::numParticles() const
{
    return m_numParticles;
}

const glowutils::AxisAlignedBoundingBox & ParticleGroup::boundingBox() const
{
    return m_particleDrawable->boundingBox();
}

float ParticleGroup::temperature() const
{
    return m_temperature;
}

void ParticleGroup::setTemperature(float temperature)
{
    m_temperature = temperature;
}

float ParticleGroup::particleSize() const
{
    return m_particleSize;
}

void ParticleGroup::setParticleSize(float size)
{
    m_particleSize = size;
    m_particleSystem->setRestParticleDistance(size);
    m_particleDrawable->setParticleSize(size);
}

physx::PxParticleFluid * ParticleGroup::particleSystem()
{
    return m_particleSystem;
}

void ParticleGroup::createParticles(const std::vector<glm::vec3> & pos, const std::vector<glm::vec3> * vel)
{
    PxU32 numParticles = static_cast<PxU32>(pos.size());
    PxU32 * indices = new PxU32[numParticles];
    if (vel) {
        assert(vel->size() == numParticles);
    }

    glowutils::AxisAlignedBoundingBox & bbox = m_particleDrawable->m_bbox;
    for (PxU32 i = 0; i < numParticles; ++i)
    {
        bbox.extend(pos.at(i));
        if (m_freeIndices.size() > 0)
        {
            indices[i] = m_freeIndices.back();
            m_freeIndices.pop_back();
        } else {
            indices[i] = m_nextFreeIndex;
            if (m_nextFreeIndex == m_lastFreeIndex) releaseOldParticles(numParticles);
            if (++m_nextFreeIndex == m_maxParticleCount) m_nextFreeIndex = 0;
        }
    }

    PxParticleCreationData particleCreationData;
    particleCreationData.numParticles = numParticles;
    particleCreationData.indexBuffer = PxStrideIterator<const PxU32>(indices);
    particleCreationData.positionBuffer = PxStrideIterator<const PxVec3>(reinterpret_cast<const PxVec3*>(pos.data()));
    if (vel)
        particleCreationData.velocityBuffer = PxStrideIterator<const PxVec3>(reinterpret_cast<const PxVec3*>(vel->data()), 0);

    bool success = m_particleSystem->createParticles(particleCreationData);
    m_numParticles += numParticles;

    if (!success)
        glow::warning("ParticleGroup::createParticles creation of %; physx particles failed", numParticles);

    delete[] indices;
}

void ParticleGroup::releaseOldParticles(const uint32_t numParticles)
{
    std::vector<uint32_t> indices;
    for (uint32_t i = 0; i < numParticles; ++i)
    {
        if (++m_lastFreeIndex == m_maxParticleCount)
            m_lastFreeIndex = 0;
        indices.push_back(m_lastFreeIndex);
    }
    PxStrideIterator<const PxU32> indexBuffer(indices.data());

    m_particleSystem->releaseParticles(numParticles, indexBuffer);
    m_numParticles -= numParticles;
}

void ParticleGroup::releaseParticles(const std::vector<uint32_t> & indices)
{
    PxU32 numParticles = static_cast<PxU32>(indices.size());

    for (PxU32 i = 0; i < numParticles; ++i)
    {
        m_freeIndices.push_back(indices.at(i));
    }

    PxStrideIterator<const PxU32> indexBuffer(indices.data());

    m_particleSystem->releaseParticles(numParticles, indexBuffer);
    m_numParticles -= numParticles;
}

uint32_t ParticleGroup::releaseParticles(const glowutils::AxisAlignedBoundingBox & boundingBox)
{
    std::vector<uint32_t> releaseIndices;
    particleIndicesInVolume(boundingBox, releaseIndices);

    releaseParticles(releaseIndices);

    assert(releaseIndices.size() < std::numeric_limits<uint32_t>::max());

    return static_cast<uint32_t>(releaseIndices.size());
}

void ParticleGroup::releaseParticlesGetPositions(const glowutils::AxisAlignedBoundingBox & boundingBox, std::vector<glm::vec3> & releasedPositions, glowutils::AxisAlignedBoundingBox & releasedBounds)
{
    std::vector<uint32_t> releaseIndices;

    PxParticleReadData * readData = m_particleSystem->lockParticleReadData();
    assert(readData);

    PxStrideIterator<const PxVec3> pxPositionIt = readData->positionBuffer;
    PxStrideIterator<const PxParticleFlags> pxFlagIt = readData->flagsBuffer;

    for (unsigned i = 0; i < readData->validParticleRange; ++i, ++pxPositionIt, ++pxFlagIt) {
        assert(pxPositionIt.ptr());
        if (!(*pxFlagIt & PxParticleFlag::eVALID))
            continue;
        const glm::vec3 & pos = reinterpret_cast<const glm::vec3&>(*pxPositionIt.ptr());
        if (!boundingBox.inside(pos))
            continue;
        releasedPositions.push_back(pos);
        releaseIndices.push_back(i);
        releasedBounds.extend(pos);
    }

    readData->unlock();

    releaseParticles(releaseIndices);
}

void ParticleGroup::createParticle(const glm::vec3 & position, const glm::vec3 & velocity)
{
    std::vector<glm::vec3> vel({ velocity });
    createParticles({ position }, &vel);
}

void ParticleGroup::setImmutableProperties(const ImmutableParticleProperties & properties)
{
    setImmutableProperties(properties.maxMotionDistance, properties.gridSize, properties.restOffset, properties.contactOffset, properties.restParticleDistance);
}

void ParticleGroup::setMutableProperties(const MutableParticleProperties & properties)
{
    setMutableProperties(properties.restitution, properties.dynamicFriction, properties.staticFriction, properties.damping, properties.externalAcceleration, properties.particleMass, properties.viscosity, properties.stiffness);
}

void ParticleGroup::setImmutableProperties(const physx::PxReal maxMotionDistance, const physx::PxReal gridSize, const physx::PxReal restOffset, const physx::PxReal contactOffset, const physx::PxReal restParticleDistance)
{
    assert(m_particleSystem);
    assert(m_scene);

    m_immutableProperties.maxMotionDistance = maxMotionDistance;
    m_immutableProperties.gridSize = gridSize;
    m_immutableProperties.restOffset = restOffset;
    m_immutableProperties.contactOffset = contactOffset;
    m_immutableProperties.restParticleDistance = restParticleDistance;

    PxSceneWriteLock scopedLock(* m_scene);

    m_scene->removeActor(*m_particleSystem);

    m_particleSystem->setMaxMotionDistance(maxMotionDistance);
    m_particleSystem->setGridSize(gridSize);
    m_particleSystem->setRestOffset(restOffset);
    m_particleSystem->setContactOffset(contactOffset);
    m_particleSystem->setRestParticleDistance(restParticleDistance);
    setParticleSize(restParticleDistance);

    m_scene->addActor(*m_particleSystem);
}

void ParticleGroup::setMutableProperties(const physx::PxReal restitution, const physx::PxReal dynamicFriction, const physx::PxReal staticFriction, const physx::PxReal damping, const glm::vec3 &externalAcceleration, const physx::PxReal particleMass, const physx::PxReal viscosity, const physx::PxReal stiffness)
{
    m_mutableProperties.restitution = restitution;
    m_mutableProperties.dynamicFriction = dynamicFriction;
    m_mutableProperties.staticFriction = staticFriction;
    m_mutableProperties.damping = damping;
    m_mutableProperties.externalAcceleration = externalAcceleration;
    m_mutableProperties.particleMass = particleMass;
    m_mutableProperties.viscosity = viscosity;
    m_mutableProperties.stiffness = stiffness;

    m_particleSystem->setRestitution(restitution);
    m_particleSystem->setDynamicFriction(dynamicFriction);
    m_particleSystem->setStaticFriction(staticFriction);
    m_particleSystem->setDamping(damping);
    m_particleSystem->setExternalAcceleration(reinterpret_cast<const physx::PxVec3&>(externalAcceleration));
    m_particleSystem->setParticleMass(particleMass);
    m_particleSystem->setViscosity(viscosity);
    m_particleSystem->setStiffness(stiffness);
}

void ParticleGroup::setUseGpuParticles(const bool enable)
{
    if (m_gpuParticles == enable) return;

    m_gpuParticles = enable;

    assert(m_particleSystem);
    assert(m_scene);

    m_scene->removeActor(*m_particleSystem);

    m_particleSystem->setParticleBaseFlag(physx::PxParticleBaseFlag::eGPU, m_gpuParticles);

    m_scene->addActor(*m_particleSystem);
}

bool ParticleGroup::useGpuParticles() const
{
    return m_gpuParticles;
}

void ParticleGroup::updatePhysics(double /*delta*/)
{
    if (m_numParticles == 0) {
        stopSound();
    }
    else {
        startSound();
    }
}

void ParticleGroup::updateVisuals()
{
    if (m_numParticles > 0)
        SoundManager::instance()->setSoundPosition(m_soundChannel, m_particleDrawable->boundingBox().center());
}

void ParticleGroup::giveGiftTo(ParticleGroup & other)
{
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> velocities;

    PxParticleReadData * readData = m_particleSystem->lockParticleReadData();
    assert(readData);

    PxStrideIterator<const PxVec3> pxPositionIt = readData->positionBuffer;
    PxStrideIterator<const PxParticleFlags> pxFlagIt = readData->flagsBuffer;
    PxStrideIterator<const PxVec3> pxVelocityIt = readData->velocityBuffer;

    for (unsigned i = 0; i < readData->validParticleRange; ++i, ++pxPositionIt, ++pxFlagIt, ++pxVelocityIt) {
        assert(pxPositionIt.ptr());
        if (!(*pxFlagIt & PxParticleFlag::eVALID))
            continue;
        const glm::vec3 & pos = reinterpret_cast<const glm::vec3&>(*pxPositionIt.ptr());
        const glm::vec3 & vel = reinterpret_cast<const glm::vec3&>(*pxVelocityIt.ptr());
        positions.push_back(pos);
        velocities.push_back(vel);
    }

    readData->unlock();

    other.createParticles(positions, &velocities);
    
    other.setTemperature((m_temperature * m_numParticles + other.m_temperature * other.m_numParticles) / (m_numParticles + other.m_numParticles));
}

void ParticleGroup::particlesInVolume(const glowutils::AxisAlignedBoundingBox & boundingBox, std::vector<glm::vec3> & particles, glowutils::AxisAlignedBoundingBox & subbox) const
{
    PxParticleReadData * readData = m_particleSystem->lockParticleReadData();
    assert(readData);

    PxStrideIterator<const PxVec3> pxPositionIt = readData->positionBuffer;
    PxStrideIterator<const PxParticleFlags> pxFlagIt = readData->flagsBuffer;

    subbox = glowutils::AxisAlignedBoundingBox();

    for (unsigned i = 0; i < readData->validParticleRange; ++i, ++pxPositionIt, ++pxFlagIt) {
        assert(pxPositionIt.ptr());
        if (!(*pxFlagIt & PxParticleFlag::eVALID))
            continue;
        const glm::vec3 & pos = reinterpret_cast<const glm::vec3&>(*pxPositionIt.ptr());
        if (!boundingBox.inside(pos))
            continue;
        particles.push_back(pos);
        subbox.extend(pos);
    }

    readData->unlock();
}

void ParticleGroup::particleIndicesInVolume(const glowutils::AxisAlignedBoundingBox & boundingBox, std::vector<uint32_t> & particleIndices) const
{
    PxParticleReadData * readData = m_particleSystem->lockParticleReadData();
    assert(readData);

    PxStrideIterator<const PxVec3> pxPositionIt = readData->positionBuffer;
    PxStrideIterator<const PxParticleFlags> pxFlagIt = readData->flagsBuffer;

    for (unsigned i = 0; i < readData->validParticleRange; ++i, ++pxPositionIt, ++pxFlagIt) {
        assert(pxPositionIt.ptr());
        if (!(*pxFlagIt & PxParticleFlag::eVALID))
            continue;
        const glm::vec3 & pos = reinterpret_cast<const glm::vec3&>(*pxPositionIt.ptr());
        if (!boundingBox.inside(pos))
            continue;
        particleIndices.push_back(i);
    }

    readData->unlock();
}

void ParticleGroup::particlePositionsIndicesVelocitiesInVolume(const glowutils::AxisAlignedBoundingBox & boundingBox, std::vector<glm::vec3> & positions, std::vector<uint32_t> & particleIndices, std::vector<glm::vec3> & velocities) const
{
    PxParticleReadData * readData = m_particleSystem->lockParticleReadData();
    assert(readData);

    PxStrideIterator<const PxVec3> pxPositionIt = readData->positionBuffer;
    PxStrideIterator<const PxParticleFlags> pxFlagIt = readData->flagsBuffer;
    PxStrideIterator<const PxVec3> pxVelocityIt = readData->velocityBuffer;

    for (unsigned i = 0; i < readData->validParticleRange; ++i, ++pxPositionIt, ++pxFlagIt, ++pxVelocityIt) {
        assert(pxPositionIt.ptr());
        if (!(*pxFlagIt & PxParticleFlag::eVALID))
            continue;
        const glm::vec3 & pos = reinterpret_cast<const glm::vec3&>(*pxPositionIt.ptr());
        if (!boundingBox.inside(pos))
            continue;
        const glm::vec3 & vel = reinterpret_cast<const glm::vec3&>(*pxVelocityIt.ptr());
        positions.push_back(pos);
        particleIndices.push_back(i);
        velocities.push_back(vel);
    }

    readData->unlock();
}


void ParticleGroup::updateSounds(bool isWorldPaused)
{
    if (isWorldPaused)
    {
        stopSound();
        m_wasSoundPlaying = true;
    }
    else if (m_wasSoundPlaying)
    {
        startSound();
    }
}

void ParticleGroup::startSound()
{
    SoundManager::instance()->setPaused(m_soundChannel, false);
    m_wasSoundPlaying = true;
}

void ParticleGroup::stopSound()
{
    SoundManager::instance()->setPaused(m_soundChannel, true);
    m_wasSoundPlaying = false;
}
