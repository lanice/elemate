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

    glowutils::AxisAlignedBoundingBox intersectVolume;
    const glm::vec3 & i_llf = intersectVolume.llf();    // to shorten the lua function call
    const glm::vec3 & i_urb = intersectVolume.urb();

    debug_intersectionBoxes.clear();

    // this .. tends to be ...slooooow
    for (auto leftHand = particleGroups.cbegin(); leftHand != lastLeftHand; ++leftHand) {
        auto rightHand = leftHand;
        ++rightHand;
        for (; rightHand != lastRightHand; ++rightHand) {
            if (checkBoundingBoxCollision(leftHand->second->boundingBox(), rightHand->second->boundingBox(), &intersectVolume)) {

                m_lua->call("particleBboxCollision", leftHand->first, rightHand->first, i_llf.x, i_llf.y, i_llf.z, i_urb.x, i_urb.y, i_urb.z);
                
                debug_intersectionBoxes.push_back(IntersectionBox(i_llf, i_urb));
            }
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

ParticleCollision::IntersectionBox::IntersectionBox(const glm::vec3 & llf, const glm::vec3 & urb)
: llf(llf)
, urb(urb)
{
}
