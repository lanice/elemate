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

void ParticleCollision::particleGroupDeleted(const std::string & elementName, int /*id*/)
{
    m_particleGroupIds.erase(elementName);
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

            // first: check if the bounding boxes intersect (it's fast, as we already have the boxes)
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

            // get the particles of one group that are inside the sub-box of the other group
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

            // now do the more complex work: tree based collision check, to detect collisions at multiple independent places
            treeCheck(commonSubbox, leftMinimalParticleSubset, rightMinimalParticleSubset, 20);


            // (this sets the size to 0, but doesn't free the reserved memory)
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

void ParticleCollision::treeCheck(const glowutils::AxisAlignedBoundingBox & volume, const std::vector<glm::vec3> & leftHandPositions, const std::vector<glm::vec3> & rightHandPositions, int depth)
{

    // recursion end

    if (leftHandPositions.empty() || rightHandPositions.empty()) {
        // nothing to do here, having only particles of one kind (or none)
        return;
    }

    //!!! AxisAlignedBoundingBox center and radius don't do what we want here.. they seem.. wrong


    glm::vec3 dimensions = volume.urb() - volume.llf();

    float maxLength = std::max(dimensions.x, std::max(dimensions.y, dimensions.z));

    if (maxLength < 0.3f || depth <= 0) {   // magic number: minimal size of the box for tree based check

        if (depth == 0) {
            glow::debug("treeCheckEnd, boxSize: %; (maxRecursionDepth)", maxLength);
        }
        else
            glow::debug("treeCheckEnd, boxSize: %; (low box size)", maxLength);
        std::vector<glm::vec3> leftReleasedPositions;
        std::vector<glm::vec3> rightReleasedPositions;

        m_currentLeftHand->releaseParticlesGetPositions(volume, leftReleasedPositions);
        m_currentRightHand->releaseParticlesGetPositions(volume, rightReleasedPositions);

        glow::debug("deleting %; particles", leftReleasedPositions.size() + rightReleasedPositions.size());

        std::string reaction = m_lua->call<std::string>("elementReaction", m_currentLeftHand->elementName(), m_currentRightHand->elementName(), leftReleasedPositions.size(), rightReleasedPositions.size());
        ParticleGroup * newGroup = particleGroup(reaction);

        newGroup->createParticles(leftReleasedPositions);
        newGroup->createParticles(rightReleasedPositions);

        debug_intersectionBoxes.push_back(IntersectionBox(volume.llf(), volume.urb()));

        return;
    }

    // still particles in the current box, and box is too large -> recursive split

    float splitValue = std::numeric_limits<float>::max();
    int splitAxis = -1;

    // split along the longest axis
    if (dimensions.x > dimensions.y) {
        if (dimensions.x > dimensions.z) { // x-split
            splitValue = (volume.urb().x - volume.llf().x) * 0.5f + volume.llf().x;
            splitAxis = 0;
        }
    }
    else {
        if (dimensions.y > dimensions.z) { // y-split
            splitValue = (volume.urb().y - volume.llf().y) * 0.5f + volume.llf().y;
            splitAxis = 1;
        }
    }
    if (splitAxis == -1) {  // z-split
        splitValue = (volume.urb().z - volume.llf().x) * 0.5f + volume.llf().z;
        splitAxis = 2;
    }
    assert(splitValue < std::numeric_limits<float>::max());

    // split the left hand and right hand particle lists into two special groups
    // group 1 for the smaller coordinates, group 2 for the greater ones
    std::vector<glm::vec3> group1LeftHand;
    std::vector<glm::vec3> group1RightHand;
    std::vector<glm::vec3> group2LeftHand;
    std::vector<glm::vec3> group2RightHand;

    glowutils::AxisAlignedBoundingBox bboxGroup1;
    glowutils::AxisAlignedBoundingBox bboxGroup2;

    for (int i = 0; i < leftHandPositions.size(); ++i) {
        const glm::vec3 & position = leftHandPositions.at(i);
        if (position[splitAxis] < splitValue) {
            group1LeftHand.push_back(position);
            bboxGroup1.extend(position);
        }
        else {
            group2LeftHand.push_back(position);
            bboxGroup2.extend(position);
        }
    }
    for (int i = 0; i < rightHandPositions.size(); ++i) {
        const glm::vec3 & position = rightHandPositions.at(i);
        if (position[splitAxis] < splitValue) {
            group1RightHand.push_back(position);
            bboxGroup1.extend(position);
        }
        else {
            group2RightHand.push_back(position);
            bboxGroup2.extend(position);
        }
    }

    treeCheck(bboxGroup1, group1LeftHand, group1RightHand, depth - 1);
    treeCheck(bboxGroup2, group2LeftHand, group2RightHand, depth - 1);
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
