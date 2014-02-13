#include "particlegroup.h"

#include <cassert>
#include <random>
#include <ctime>
#include <functional>

#include <glow/logging.h>

#include <PxPhysics.h>
#include <PxScene.h>
#include <PxSceneLock.h>

#include "rendering/particledrawable.h"
#include "io/soundmanager.h"

namespace {
    std::mt19937 rng;
}

using namespace physx;

namespace {
    uint32_t seed_val = static_cast<uint32_t>(std::time(0));
    bool initRng() {
        rng.seed(seed_val);
        return true;
    }
    bool didRngInit = initRng();
}

ParticleGroup::ParticleGroup(
    const std::string & elementName,
    const bool enableGpuParticles,
    const PxU32 maxParticleCount,
    const ImmutableParticleProperties & immutableProperties,
    const MutableParticleProperties & mutableProperties
    )
: m_particleSystem(nullptr)
, m_scene(nullptr)
, m_elementName(elementName)
, m_particleDrawable(std::make_shared<ParticleDrawable>(elementName, maxParticleCount))
, m_maxParticleCount(maxParticleCount)
, m_indices(new PxU32[maxParticleCount]())
, m_nextFreeIndex(0)
, m_lastFreeIndex(maxParticleCount-1)
, m_emitting(false)
, m_timeSinceLastEmit(0.0)
, m_gpuParticles(enableGpuParticles)
{
    for (PxU32 i = 0; i < maxParticleCount; ++i) m_indices[i] = i;

    assert(PxGetPhysics().getNbScenes() == 1);
    PxScene * pxScenePtrs[1];
    PxGetPhysics().getScenes(pxScenePtrs, 1);
    m_scene = pxScenePtrs[0];

    PxSceneWriteLock scopedLock(* m_scene);

    m_particleSystem = PxGetPhysics().createParticleFluid(maxParticleCount, false);
    assert(m_particleSystem);
    m_particleSystem->setParticleBaseFlag(physx::PxParticleBaseFlag::eGPU, m_gpuParticles);

    m_scene->addActor(*m_particleSystem);
    
    setImmutableProperties(immutableProperties);
    setMutableProperties(mutableProperties);
}

ParticleGroup::~ParticleGroup()
{
    PxSceneWriteLock scopedLock(* m_scene);

    m_particleSystem->releaseParticles();
    m_scene->removeActor(*m_particleSystem);
    m_particleSystem = nullptr;
}

const std::string & ParticleGroup::elementName() const
{
    return m_elementName;
}

const glowutils::AxisAlignedBoundingBox & ParticleGroup::boundingBox() const
{
    return m_particleDrawable->boundingBox();
}

float ParticleGroup::particleSize() const
{
    return m_particleSystem->getRestParticleDistance();
}

physx::PxParticleFluid * ParticleGroup::particleSystem()
{
    return m_particleSystem;
}

void ParticleGroup::createParticles(const std::vector<glm::vec3> & pos, const std::vector<glm::vec3> * vel)
{
    PxU32 numParticles = static_cast<PxU32>(pos.size());
    PxU32 * indices = new PxU32[numParticles];
    PxVec3 * positions = new PxVec3[numParticles];
    PxVec3 * velocities = nullptr;
    if (vel) {
        assert(vel->size() == numParticles);
        velocities = new PxVec3[numParticles];
    }

    for (PxU32 i = 0; i < numParticles; ++i)
    {
        if (m_freeIndices.size() > 0)
        {
            indices[i] = m_freeIndices.back();
            m_freeIndices.pop_back();
        } else {
            indices[i] = m_nextFreeIndex;
            if (m_nextFreeIndex == m_lastFreeIndex) releaseOldParticles(numParticles);
            if (++m_nextFreeIndex == m_maxParticleCount) m_nextFreeIndex = 0;
        }

        positions[i] = PxVec3(pos.at(i).x, pos.at(i).y, pos.at(i).z);
    }
    if (vel) {
        for (PxU32 i = 0; i < numParticles; ++i)
            velocities[i] = PxVec3(vel->at(i).x, vel->at(i).y, vel->at(i).z);
    }

    PxParticleCreationData particleCreationData;
    particleCreationData.numParticles = numParticles;
    particleCreationData.indexBuffer = PxStrideIterator<const PxU32>(indices);
    particleCreationData.positionBuffer = PxStrideIterator<const PxVec3>(positions);
    if (vel)
        particleCreationData.velocityBuffer = PxStrideIterator<const PxVec3>(velocities, 0);

    bool success = m_particleSystem->createParticles(particleCreationData);

    if (!success)
        glow::warning("ParticleGroup::createParticles creation of %; physx particles failed", numParticles);

    delete[] indices;
    delete[] positions;
    delete[] velocities;
}

void ParticleGroup::releaseOldParticles(const uint32_t numParticles)
{
    std::vector<uint32_t> indices;
    for (uint32_t i = 0; i < numParticles; ++i)
    {
        if (++m_lastFreeIndex == m_maxParticleCount) m_lastFreeIndex = 0;
        indices.push_back(m_lastFreeIndex);
    }

    PxU32 * indexBuffer = new PxU32[numParticles];

    for (PxU32 i = 0; i < numParticles; ++i)
    {
        PxU32 index = indices.at(i);
        indexBuffer[i] = index;
    }

    PxStrideIterator<const PxU32> indexxBuffer(indexBuffer);

    m_particleSystem->releaseParticles(numParticles, indexxBuffer);

    delete[] indexBuffer;
}

void ParticleGroup::releaseParticles(const std::vector<uint32_t> & indices)
{
    PxU32 numParticles = static_cast<PxU32>(indices.size());
    PxU32 * indexBuffer = new PxU32[numParticles];

    for (PxU32 i = 0; i < numParticles; ++i)
    {
        PxU32 index = indices.at(i);
        indexBuffer[i] = index;
        m_freeIndices.push_back(index);
    }

    PxStrideIterator<const PxU32> indexxBuffer(indexBuffer);

    m_particleSystem->releaseParticles(numParticles, indexxBuffer);

    delete[] indexBuffer;
}

uint32_t ParticleGroup::releaseParticles(const glowutils::AxisAlignedBoundingBox & boundingBox)
{
    std::vector<uint32_t> releaseIndices;
    particleIndicesInVolume(boundingBox, releaseIndices);

    releaseParticles(releaseIndices);

    assert(releaseIndices.size() < std::numeric_limits<uint32_t>::max());

    return static_cast<uint32_t>(releaseIndices.size());
}

void ParticleGroup::releaseParticlesGetPositions(const glowutils::AxisAlignedBoundingBox & boundingBox, std::vector<glm::vec3> & releasedPositions)
{
    std::vector<uint32_t> releaseIndices;

    particlePositionsIndicesInVolume(boundingBox, releasedPositions, releaseIndices);

    releaseParticles(releaseIndices);
}

void ParticleGroup::createParticle(const glm::vec3 & position, const glm::vec3 & velocity)
{
    std::vector<glm::vec3> vel({ velocity });
    createParticles({ position }, &vel);
}

void ParticleGroup::emit(const float ratio, const glm::vec3 & position, const glm::vec3 & direction)
{
    m_emitRatio = ratio;
    m_emitPosition = position;
    m_emitDirection = glm::normalize(direction);
    m_emitting = true;

	startSound();
}

void ParticleGroup::stopEmit()
{
    m_emitting = false;
    m_timeSinceLastEmit = 0.0;

	stopSound();
}

void ParticleGroup::updateEmitting(const double & delta)
{
    if (!m_emitting) return;

    unsigned int particlesToEmit = static_cast<unsigned int>(glm::floor(m_emitRatio * delta));

    std::uniform_real_distribution<float> uniform_dist(-0.75f, 0.75f);
    std::function<float()> scatterFactor = [&](){ return uniform_dist(rng); };

    if (particlesToEmit > 0)
    {
        for (unsigned int i = 0; i < particlesToEmit; ++i)
            createParticle(m_emitPosition, glm::vec3((m_emitDirection.x + scatterFactor()), (m_emitDirection.y + scatterFactor()), (m_emitDirection.z + scatterFactor())) * 100.f);

        m_timeSinceLastEmit = 0.0;
    } else {
        if (m_timeSinceLastEmit >= 1.0 / m_emitRatio)
        {
            createParticle(m_emitPosition, glm::vec3((m_emitDirection.x + scatterFactor()), (m_emitDirection.y + scatterFactor()), (m_emitDirection.z + scatterFactor())) * 100.f);
            m_timeSinceLastEmit = 0.0;
        } else {
            m_timeSinceLastEmit += delta;
        }
    }

	FMOD_VECTOR soundPos;
	soundPos.x = m_emitPosition.x;
	soundPos.y = m_emitPosition.y;
	soundPos.z = m_emitPosition.z;
	m_soundManager->setSoundPos(m_soundChannel, soundPos);
}

glm::vec3 vec3(const physx::PxVec3 & vec3)
{
    return glm::vec3(vec3.x, vec3.y, vec3.z);
}

void ParticleGroup::updateVisuals()
{
    PxParticleReadData * readData = m_particleSystem->lockParticleReadData();
    assert(readData);

    m_particleDrawable->updateParticles(readData);

    // Get drained Particles
    std::vector<uint32_t> indices;
    PxStrideIterator<const PxParticleFlags> flagsIt(readData->flagsBuffer);
    PxStrideIterator<const PxVec3> positionIt = readData->positionBuffer;

    glowutils::AxisAlignedBoundingBox bbox;

    for (unsigned i = 0; i < readData->validParticleRange; ++i, ++flagsIt, ++positionIt) {
        if (*flagsIt & PxParticleFlag::eCOLLISION_WITH_DRAIN)
            indices.push_back(i);
        if (*flagsIt & PxParticleFlag::eVALID) {
            bbox.extend(vec3(*positionIt));
        }
    }

    readData->unlock();

    m_particleDrawable->setParticleSize(m_particleSystem->getRestParticleDistance());
    m_particleDrawable->setBoudingBox(bbox);

    releaseParticles(indices);
}

void ParticleGroup::setImmutableProperties(const ImmutableParticleProperties & properties)
{
    setImmutableProperties(properties.maxMotionDistance, properties.gridSize, properties.restOffset, properties.contactOffset, properties.restParticleDistance);
}

void ParticleGroup::setMutableProperties(const MutableParticleProperties & properties)
{
    setMutableProperties(properties.restitution, properties.dynamicFriction, properties.staticFriction, properties.damping, /*properties.externalAcceleration,*/ properties.particleMass, properties.viscosity, properties.stiffness);
}

void ParticleGroup::setImmutableProperties(const physx::PxReal maxMotionDistance, const physx::PxReal gridSize, const physx::PxReal restOffset, const physx::PxReal contactOffset, const physx::PxReal restParticleDistance)
{
    assert(m_particleSystem);
    assert(m_scene);

    PxSceneWriteLock scopedLock(* m_scene);

    m_scene->removeActor(*m_particleSystem);

    m_particleSystem->setMaxMotionDistance(maxMotionDistance);
    m_particleSystem->setGridSize(gridSize);
    m_particleSystem->setRestOffset(restOffset);
    m_particleSystem->setContactOffset(contactOffset);
    m_particleSystem->setRestParticleDistance(restParticleDistance);

    m_scene->addActor(*m_particleSystem);
}

void ParticleGroup::setMutableProperties(const physx::PxReal restitution, const physx::PxReal dynamicFriction, const physx::PxReal staticFriction, const physx::PxReal damping, /*const physx::PxVec3 externalAcceleration,*/ const physx::PxReal particleMass, const physx::PxReal viscosity, const physx::PxReal stiffness)
{
    m_particleSystem->setRestitution(restitution);
    m_particleSystem->setDynamicFriction(dynamicFriction);
    m_particleSystem->setStaticFriction(staticFriction);
    m_particleSystem->setDamping(damping);
    // m_particleSystem->setExternalAcceleration(externalAcceleration);
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
        const physx::PxVec3 & pxPosition = *pxPositionIt;
        const glm::vec3 pos = glm::vec3(pxPosition.x, pxPosition.y, pxPosition.z);
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
        const physx::PxVec3 & pxPosition = *pxPositionIt;
        const glm::vec3 pos = glm::vec3(pxPosition.x, pxPosition.y, pxPosition.z);
        if (!boundingBox.inside(pos))
            continue;
        particleIndices.push_back(i);
    }

    readData->unlock();
}

void ParticleGroup::particlePositionsIndicesInVolume(const glowutils::AxisAlignedBoundingBox & boundingBox, std::vector<glm::vec3> & positions, std::vector<uint32_t> & particleIndices) const
{
    PxParticleReadData * readData = m_particleSystem->lockParticleReadData();
    assert(readData);

    PxStrideIterator<const PxVec3> pxPositionIt = readData->positionBuffer;
    PxStrideIterator<const PxParticleFlags> pxFlagIt = readData->flagsBuffer;

    for (unsigned i = 0; i < readData->validParticleRange; ++i, ++pxPositionIt, ++pxFlagIt) {
        assert(pxPositionIt.ptr());
        if (!(*pxFlagIt & PxParticleFlag::eVALID))
            continue;
        const physx::PxVec3 & pxPosition = *pxPositionIt;
        const glm::vec3 pos = glm::vec3(pxPosition.x, pxPosition.y, pxPosition.z);
        if (!boundingBox.inside(pos))
            continue;
        positions.push_back(pos);
        particleIndices.push_back(i);
    }

    readData->unlock();
}

void ParticleGroup::passSoundManager(std::shared_ptr<SoundManager> sound_manager)
{
	m_soundManager = sound_manager;
	m_soundChannel = m_soundManager->createNewChannel("data/sounds/elements/" + m_elementName + ".wav", true, true, true);
	m_soundManager->setVolume(m_soundChannel, 0.15f);
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
	m_soundManager->setPaused(m_soundChannel, false);
	m_wasSoundPlaying = true;
}

void ParticleGroup::stopSound()
{
	m_soundManager->setPaused(m_soundChannel, true);
	m_wasSoundPlaying = false;
}