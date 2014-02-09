#include "particlecollision.h"

#include <algorithm>

#include <glowutils/AxisAlignedBoundingBox.h>
#include <glow/logging.h>

#include "particlescriptaccess.h"
#include "particlegroup.h"
#include "lua/luawrapper.h"

std::list<ParticleCollision::IntersectionBox> ParticleCollision::debug_intersectionBoxes;

ParticleCollision::ParticleCollision(ParticleScriptAccess & psa)
: m_psa(psa)
, m_lua(new LuaWrapper())
{
    m_lua->loadScript("scripts/collision.lua");
    m_psa.registerLuaFunctions(m_lua);
}

ParticleCollision::~ParticleCollision()
{
    delete m_lua;
}

void ParticleCollision::performCheck()
{
    const auto & particleGroups = m_psa.m_particleGroups;

    if (particleGroups.size() < 2)
        return;

    auto lastLeftHand = --particleGroups.cend();
    auto lastRightHand = particleGroups.cend();

    glowutils::AxisAlignedBoundingBox intersectVolume; // the intersection volume of the particle group bounding boxes

    glowutils::AxisAlignedBoundingBox leftSubbox;   // the subvolume of the left hand group that actually contains particles
    glowutils::AxisAlignedBoundingBox rightSubbox;
    std::vector<glm::vec3> leftParticleSubset;
    std::vector<glm::vec3> rightParticleSubset;

    glowutils::AxisAlignedBoundingBox commonSubbox;

    debug_intersectionBoxes.clear();

    // this .. tends to be ...slooooow
    for (auto leftHand = particleGroups.cbegin(); leftHand != lastLeftHand; ++leftHand) {
        auto rightHand = leftHand;
        ++rightHand;
        for (; rightHand != lastRightHand; ++rightHand) {
            ParticleGroup & leftGroup = *leftHand->second;
            ParticleGroup & rightGroup = *rightHand->second;

            // first: check if the boundingboxes intersect (it's fast, as we already have the boxes)
            if (!checkBoundingBoxCollision(leftGroup.boundingBox(), rightGroup.boundingBox(), &intersectVolume))
                continue; // not interested if the groups don't intersect

            // now ask the scripts if this intersection may be interesting
            if (!m_lua->call<bool>("collisionCheckRelevance", leftHand->first, rightHand->first))
                continue;

            // get some more information out of the bounding boxes: this requires iteration over the particles groups
            leftGroup.particlesInVolume(intersectVolume, leftParticleSubset, leftSubbox);
            rightGroup.particlesInVolume(intersectVolume, rightParticleSubset, rightSubbox);

            if (leftParticleSubset.empty() || rightParticleSubset.empty())
                continue;   // particles of one box flow into the other, where the other doesn't have particles

            commonSubbox = glowutils::AxisAlignedBoundingBox(); // reset the box
            commonSubbox.extend(glm::max(leftSubbox.llf(), rightSubbox.llf()));
            commonSubbox.extend(glm::min(leftSubbox.urb(), rightSubbox.urb()));

            debug_intersectionBoxes.push_back(IntersectionBox(commonSubbox.llf(), commonSubbox.urb()));

            leftParticleSubset.clear();
            rightParticleSubset.clear();
            leftGroup.releaseParticlesGetPositions(commonSubbox, leftParticleSubset);
            rightGroup.releaseParticlesGetPositions(commonSubbox, rightParticleSubset);

            std::string reaction = m_lua->call<std::string>("elementReaction", leftGroup.elementName(), rightGroup.elementName(), leftParticleSubset.size(), rightParticleSubset.size());

            particleGroup(reaction)->createParticles(leftParticleSubset);
            particleGroup(reaction)->createParticles(rightParticleSubset);
        }
    }
}

bool ParticleCollision::checkBoundingBoxCollision(const glowutils::AxisAlignedBoundingBox & box1, const glowutils::AxisAlignedBoundingBox & box2, glowutils::AxisAlignedBoundingBox * intersectVolume)
{
    float maxLeftX = std::max(box1.llf().x, box2.llf().x);
    float minRightX = std::min(box1.urb().x, box2.urb().x);

    // if the boxes don't share some space along the x axis, then they don't collide at all
    if (maxLeftX > minRightX)
        return false;

    float maxFrontZ = std::max(box1.llf().z, box2.llf().z);
    float minBackZ = std::min(box1.urb().z, box2.urb().z);

    if (maxFrontZ > minBackZ)
        return false;

    float maxBottomY = std::max(box1.llf().y, box2.llf().y);
    float minTopY = std::min(box1.urb().y, box2.urb().y);

    if (maxBottomY > minTopY)
        return false;

    // the aabb's collide of the share same values along all axis
    if (intersectVolume) {
        *intersectVolume = glowutils::AxisAlignedBoundingBox(); // reset the bbox by overwriting the old object..
        intersectVolume->extend(glm::vec3(maxLeftX, maxBottomY, maxFrontZ));
        intersectVolume->extend(glm::vec3(minRightX, minTopY, minBackZ));
    }

    return true;
}

int ParticleCollision::particleGroupId(const std::string & elementName)
{
    const auto it = m_particleGroupIds.find(elementName);
    if (it != m_particleGroupIds.end())
        return it->second;

    int newId = m_psa.createParticleGroup(elementName);
    m_particleGroupIds.emplace(elementName, newId);
    return newId;
}

ParticleGroup * ParticleCollision::particleGroup(const std::string & elementName)
{
    return m_psa.particleGroup(particleGroupId(elementName));
}

ParticleCollision::IntersectionBox::IntersectionBox(const glm::vec3 & llf, const glm::vec3 & urb)
: llf(llf)
, urb(urb)
{
}
