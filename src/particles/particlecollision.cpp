#include "particlecollision.h"

#include <algorithm>

#include <glowutils/AxisAlignedBoundingBox.h>
#include <glow/logging.h>

#include "particlescriptaccess.h"
#include "particlegroup.h"
#include "lua/luawrapper.h"

std::list<ParticleCollision::IntersectionBox> ParticleCollision::debug_intersectionBoxes;

namespace {

void extractPointsInside(const std::vector<glm::vec3> & points, const glowutils::AxisAlignedBoundingBox & box,
    std::vector<glm::vec3> & extractedPoints, glowutils::AxisAlignedBoundingBox & extractedBox)
{
    for (const glm::vec3 & point : points) {
        if (!box.inside(point))
            continue;

        extractedBox.extend(point);
        extractedPoints.push_back(point);
    }
}

}

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

    std::vector<glm::vec3> leftParticleSubset;      // the subset of particles that is inside of the intersection box
    std::vector<glm::vec3> rightParticleSubset;
    glowutils::AxisAlignedBoundingBox leftSubbox;   // the bounding box of these particles
    glowutils::AxisAlignedBoundingBox rightSubbox;
    std::vector<glm::vec3> leftMinimalParticleSubset;   // particles in the intersection box that are also inside the bounding box of the interacting group
    std::vector<glm::vec3> rightMinimalParticleSubset;  // this are the particles we will touch while reacting/interacting
    glowutils::AxisAlignedBoundingBox leftMinimalSubbox;   // the bounding box of these particles
    glowutils::AxisAlignedBoundingBox rightMinimalSubbox;

    glowutils::AxisAlignedBoundingBox commonSubbox;

    debug_intersectionBoxes.clear();

    // this .. tends to be ...slooooow
    for (auto leftHand = particleGroups.cbegin(); leftHand != lastLeftHand; ++leftHand) {
        auto rightHand = leftHand;
        ++rightHand;
        for (; rightHand != lastRightHand; ++rightHand) {
            m_currentLeftHand = leftHand->second;
            m_currentRightHand = rightHand->second;

            // first: check if the boundingboxes intersect (it's fast, as we already have the boxes)
            if (!checkBoundingBoxCollision(m_currentLeftHand->boundingBox(), m_currentRightHand->boundingBox(), &intersectVolume))
                continue; // not interested if the groups don't intersect

            // now ask the scripts if this intersection may be interesting
            if (!m_lua->call<bool>("collisionCheckRelevance", leftHand->first, rightHand->first))
                continue;

            // get some more information out of the bounding boxes: this requires iteration over the particles groups
            m_currentLeftHand->particlesInVolume(intersectVolume, leftParticleSubset, leftSubbox);
            if (leftParticleSubset.empty())
                continue;   // particles of one box flow into the other, where the other doesn't have particles
            m_currentRightHand->particlesInVolume(intersectVolume, rightParticleSubset, rightSubbox);
            if (rightParticleSubset.empty())
                continue;

            // get the particles of one group that are inside the subbox of the other group
            // this way we discard particles, that are inside the intersection box, but are actually not near particles of the interacting group
            extractPointsInside(leftParticleSubset, rightSubbox, leftMinimalParticleSubset, leftMinimalSubbox);
            if (leftMinimalParticleSubset.empty())
                continue;
            extractPointsInside(rightParticleSubset, leftSubbox, rightMinimalParticleSubset, rightMinimalSubbox);
            if (rightMinimalParticleSubset.empty())
                continue;

            // use the bounding box containing these 'minimal' point subsets for further intersection tests
            commonSubbox.extend(glm::min(leftMinimalSubbox.llf(), rightMinimalSubbox.llf()));
            commonSubbox.extend(glm::max(leftMinimalSubbox.urb(), rightMinimalSubbox.urb()));

            debug_intersectionBoxes.push_back(IntersectionBox(commonSubbox.llf(), commonSubbox.urb()));

            m_currentLeftHand->releaseParticles(leftMinimalSubbox);
            m_currentRightHand->releaseParticles(rightMinimalSubbox);

            std::string reaction = m_lua->call<std::string>("elementReaction", m_currentLeftHand->elementName(), m_currentRightHand->elementName(), leftParticleSubset.size(), rightParticleSubset.size());

            particleGroup(reaction)->createParticles(leftMinimalParticleSubset);
            particleGroup(reaction)->createParticles(rightMinimalParticleSubset);

            // (this sets the size to 0, but dosn't free the reserved memory)
            leftParticleSubset.clear();
            rightParticleSubset.clear();
            leftMinimalParticleSubset.clear();
            rightMinimalParticleSubset.clear();

            commonSubbox = glowutils::AxisAlignedBoundingBox(); // reset the box
            leftSubbox = glowutils::AxisAlignedBoundingBox();
            rightSubbox = glowutils::AxisAlignedBoundingBox();
            leftMinimalSubbox = glowutils::AxisAlignedBoundingBox();
            rightMinimalSubbox = glowutils::AxisAlignedBoundingBox();
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

    // the aabb's collide if they share same values along all axis

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
