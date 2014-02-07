#include "particlecollision.h"

#include <algorithm>

#include <glowutils/AxisAlignedBoundingBox.h>
#include <glow/logging.h>

#include "particlescriptaccess.h"
#include "particlegroup.h"

ParticleCollision::ParticleCollision(ParticleScriptAccess & psa)
: m_psa(psa)
{
}

void ParticleCollision::performCheck()
{
    const auto & particleGroups = m_psa.m_particleGroups;

    if (particleGroups.size() < 2)
        return;

    auto lastLeftHand = --particleGroups.cend();
    auto lastRightHand = particleGroups.cend();

    for (auto leftHand = particleGroups.cbegin(); leftHand != lastLeftHand; ++leftHand) {
        auto rightHand = leftHand;
        ++rightHand;
        for (; rightHand != lastRightHand; ++rightHand) {
            if (checkBoundingBoxCollision(leftHand->second->boundingBox(), rightHand->second->boundingBox())) {
                glow::debug("Collision detected: %;:%; - %;:%;",
                    leftHand->first, leftHand->second->elementName(),
                    rightHand->first, rightHand->second->elementName());
            }
        }
    }
}

bool ParticleCollision::checkBoundingBoxCollision(const glowutils::AxisAlignedBoundingBox & box1, const glowutils::AxisAlignedBoundingBox & box2)
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

    // the aabb's collide of the share same values along all axis
    return maxBottomY < minTopY;
}
