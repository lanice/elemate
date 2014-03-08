#include "particlecollision.h"

#include <algorithm>

#include <glowutils/AxisAlignedBoundingBox.h>
#include <glow/logging.h>

#include "particlescriptaccess.h"
#include "particlegroup.h"
#include "particlehelper.h"
#include "lua/luawrapper.h"
#include "terrain/terraininteraction.h"

std::list<ParticleCollision::IntersectionBox> ParticleCollision::debug_intersectionBoxes;

using namespace glowutils;
using namespace glm;

namespace {

void extractPointsInside(const std::vector<vec3> & points, const AxisAlignedBoundingBox & box,
    std::vector<vec3> & extractedPoints, AxisAlignedBoundingBox & extractedBox)
{
    for (const vec3 & point : points) {
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
, m_terrainInteraction(new TerrainInteraction("bedrock"))
{
    m_lua->loadScript("scripts/collision.lua");
    m_psa.registerLuaFunctions(*m_lua);
    m_terrainInteraction->registerLuaFunctions(*m_lua);

    registerLuaFunctions();
}

ParticleCollision::~ParticleCollision()
{
    delete m_lua;
    delete m_terrainInteraction;
}

void ParticleCollision::registerLuaFunctions()
{
    std::function<int(int, int, glm::vec3, glm::vec3)> func0 = [=](int leftGroup, int rightGroup, glm::vec3 intersectLlf, glm::vec3 intersectUrb)
    { checkCollidedParticles(leftGroup, rightGroup, glowutils::AxisAlignedBoundingBox(intersectLlf, intersectUrb)); return 0; };
    std::function<unsigned int()> func1 = std::bind(&ParticleCollision::forgetOldParticles, this);
    std::function<unsigned int(int, glm::vec3, glm::vec3)> func2 = [=](int groupId, glm::vec3 collisionLlf, glm::vec3 collisionUrb)
    { return releaseRemeberParticles(groupId, AxisAlignedBoundingBox(collisionLlf, collisionUrb)); };
    std::function<unsigned int(int, glm::vec3, glm::vec3)> func3 = [=](int groupId, glm::vec3 collisionLlf, glm::vec3 collisionUrb)
    { return releaseForgetParticles(groupId, AxisAlignedBoundingBox(collisionLlf, collisionUrb)); };
    std::function<unsigned int(std::string)> func4 = std::bind(&ParticleCollision::createFromRemembered, this, std::placeholders::_1);
    
    m_lua->Register("pc_checkCollidedParticles", func0);
    m_lua->Register("pc_forgetOldParticles", func1);
    m_lua->Register("pc_releaseRememberParticles", func2);
    m_lua->Register("pc_releaseForgetParticles", func3);
    m_lua->Register("pc_createFromRemembered", func4);
}

void ParticleCollision::particleGroupDeleted(const std::string & elementName, int id)
{
    auto it = m_particleGroupIds.find(elementName);
    if (it != m_particleGroupIds.end() && it->second == id)
        m_particleGroupIds.erase(elementName);
}

void ParticleCollision::clearParticleGroups()
{
    m_particleGroupIds.clear();
}

void ParticleCollision::performCheck()
{
    const auto & particleGroups = m_psa.m_particleGroups;

    for (auto pair = particleGroups.cbegin(); pair != particleGroups.cend(); ++pair) {
        if (!pair->second->isDown)
            continue;

        const vec3 & center = pair->second->boundingBox().center();
        m_lua->call("temperatureCheck", pair->second->elementName(), center, pair->second->numParticles());
    }

    if (particleGroups.size() < 2)
        return;

    auto lastLeftHand = --particleGroups.cend();
    auto lastRightHand = particleGroups.cend();

    glowutils::AxisAlignedBoundingBox intersectVolume;

    debug_intersectionBoxes.clear();

    // this .. tends to be ...slooooow
    for (auto leftHand = particleGroups.cbegin(); leftHand != lastLeftHand; ++leftHand) {
        auto rightHand = leftHand;
        ++rightHand;
        for (; rightHand != lastRightHand; ++rightHand) {
            // first: check if the bounding boxes intersect (it's fast, as we already have the boxes)
            if (!checkBoundingBoxCollision(leftHand->second->boundingBox(), rightHand->second->boundingBox(), &intersectVolume))
                continue; // not interested if the groups don't intersect

            // now let the script decide what to do next
            m_lua->call("boundingBoxCollision", leftHand->first, rightHand->first, intersectVolume.llf(), intersectVolume.urb());
        }
    }
}

void ParticleCollision::checkCollidedParticles(int leftGroup, int rightGroup, const glowutils::AxisAlignedBoundingBox & intersectVolume)
{
    std::vector<vec3> leftParticleSubset;      // the subset of particles that is inside of the intersection box
    std::vector<vec3> rightParticleSubset;
    AxisAlignedBoundingBox leftSubbox;   // the bounding box of these particles
    AxisAlignedBoundingBox rightSubbox;
    std::vector<vec3> leftMinimalParticleSubset;   // particles in the intersection box that are also inside the bounding box of the interacting group
    std::vector<vec3> rightMinimalParticleSubset;  // this are the particles we will touch while reacting/interacting
    AxisAlignedBoundingBox leftMinimalSubbox;   // the bounding box of these particles
    AxisAlignedBoundingBox rightMinimalSubbox;

    AxisAlignedBoundingBox commonSubbox;

    // get some more information out of the bounding boxes: this requires iteration over the particles groups
    m_psa.particleGroup(leftGroup)->particlesInVolume(intersectVolume, leftParticleSubset, leftSubbox);
    if (leftParticleSubset.empty())
        return;   // particles of one box flow into the other, where the other doesn't have particles
    m_psa.particleGroup(rightGroup)->particlesInVolume(intersectVolume, rightParticleSubset, rightSubbox);
    if (rightParticleSubset.empty())
        return;

    // do the next steps with particles we really need to look at
    if (!extractCommonPositionBox(leftParticleSubset, rightParticleSubset, leftSubbox, rightSubbox, leftMinimalParticleSubset, rightMinimalParticleSubset, commonSubbox))
        return;

    // now do the more complex work: tree based collision check, to detect collisions at multiple independent positions
    treeCheck(commonSubbox, leftMinimalParticleSubset, rightMinimalParticleSubset, 20);
}

bool ParticleCollision::checkBoundingBoxCollision(const AxisAlignedBoundingBox & box1, const AxisAlignedBoundingBox & box2, AxisAlignedBoundingBox * intersectVolume)
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
        *intersectVolume = AxisAlignedBoundingBox(); // reset the bbox by overwriting the old object..
        intersectVolume->extend(vec3(maxLeftX, maxBottomY, maxFrontZ));
        intersectVolume->extend(vec3(minRightX, minTopY, minBackZ));
    }

    return true;
}

bool ParticleCollision::extractCommonPositionBox(const std::vector<vec3> & leftHandPositions, const std::vector<vec3> & rightHandPositions,
    const AxisAlignedBoundingBox & leftBBox, const AxisAlignedBoundingBox & rightBBox,
    std::vector<vec3> & leftHandExtracted, std::vector<vec3> & rightHandExtracted,
    glowutils::AxisAlignedBoundingBox & commonBBox)
{
    // not interested at all if the boxes don't collide
    if (!checkBoundingBoxCollision(leftBBox, rightBBox))
        return false;

    AxisAlignedBoundingBox leftSubbox;
    AxisAlignedBoundingBox rightSubbox;

    // get the points of one group that are inside the bounding box of the other group

    extractPointsInside(leftHandPositions, rightBBox, leftHandExtracted, leftSubbox);
    if (leftHandExtracted.empty())
        return false;

    extractPointsInside(rightHandPositions, leftBBox, rightHandExtracted, rightSubbox);
    if (rightHandExtracted.empty())
        return false;

    // get the bounding box containing these minimal position subsets
    commonBBox.extend(glm::min(leftSubbox.llf(), rightSubbox.llf()));
    commonBBox.extend(glm::max(leftSubbox.urb(), rightSubbox.urb()));

    return true;
}

void ParticleCollision::treeCheck(const AxisAlignedBoundingBox & volume, const std::vector<vec3> & leftHandPositions, const std::vector<vec3> & rightHandPositions, int depth)
{
    // recursion end

    if (leftHandPositions.empty() || rightHandPositions.empty()) {
        // nothing to do here, having only particles of one kind (or none)
        return;
    }

    //!!! AxisAlignedBoundingBox center and radius don't do what we want here.. they seem.. wrong


    vec3 dimensions = volume.urb() - volume.llf();

    float maxLength = std::max(dimensions.x, std::max(dimensions.y, dimensions.z));

    if (maxLength < 0.3f || depth <= 0) {   // magic number: minimal size of the box for tree based check

        // call the script to handle this particle collision
        m_lua->call("particleCollision", volume.llf(), volume.urb());

        debug_intersectionBoxes.push_back({ volume.llf(), volume.urb() });

        return;
    }

    // still particles in the current box, and box is too large -> recursive split

    float splitValue = 0;
    int splitAxis = longestAxis(volume, splitValue);

    // split the left hand and right hand particle lists into two special groups
    // group 1 for the smaller coordinates, group 2 for the greater ones
    std::vector<vec3> group1LeftHand;
    std::vector<vec3> group1RightHand;
    std::vector<vec3> group2LeftHand;
    std::vector<vec3> group2RightHand;

    AxisAlignedBoundingBox bboxGroup1Left;
    AxisAlignedBoundingBox bboxGroup1Right;
    AxisAlignedBoundingBox bboxGroup2Left;
    AxisAlignedBoundingBox bboxGroup2Right;

    // spacial splitting of the particles along the splitting plane
    for (int i = 0; i < leftHandPositions.size(); ++i) {
        const vec3 & position = leftHandPositions.at(i);
        if (position[splitAxis] < splitValue) {
            group1LeftHand.push_back(position);
            bboxGroup1Left.extend(position);
        }
        else {
            group2LeftHand.push_back(position);
            bboxGroup2Left.extend(position);
        }
    }
    for (int i = 0; i < rightHandPositions.size(); ++i) {
        const vec3 & position = rightHandPositions.at(i);
        if (position[splitAxis] < splitValue) {
            group1RightHand.push_back(position);
            bboxGroup1Right.extend(position);
        }
        else {
            group2RightHand.push_back(position);
            bboxGroup2Right.extend(position);
        }
    }

    std::vector<vec3> leftExtracted, rightExtracted;
    AxisAlignedBoundingBox commonBoundingBox;
    std::pair<int, int> a(1, 0), b(1, 0);
    // for each created splitting subset: extract the particles that may interact with the other group
    if (extractCommonPositionBox(group1LeftHand, group1RightHand, bboxGroup1Left, bboxGroup1Right, leftExtracted, rightExtracted, commonBoundingBox)) {
        // and if so, try continuing with another spacial splitting
        treeCheck(commonBoundingBox, leftExtracted, rightExtracted, depth - 1);
    }
    // same for the second splitting subset, reuse the container and bounding box objects
    leftExtracted.clear();
    rightExtracted.clear();
    commonBoundingBox = AxisAlignedBoundingBox();
    if (extractCommonPositionBox(group2LeftHand, group2RightHand, bboxGroup2Left, bboxGroup2Right, leftExtracted, rightExtracted, commonBoundingBox)) {
        treeCheck(commonBoundingBox, leftExtracted, rightExtracted, depth - 1);
    }
}

unsigned int ParticleCollision::forgetOldParticles()
{
    assert(m_remeberedParticles.size() < std::numeric_limits<unsigned int>::max());
    unsigned int num = static_cast<unsigned int>(m_remeberedParticles.size());
    m_remeberedParticles.clear();
    return num;
}

unsigned int ParticleCollision::releaseRemeberParticles(int groupId, const AxisAlignedBoundingBox & volume)
{
    unsigned int nbParticlesBefore = static_cast<unsigned int>(m_remeberedParticles.size());
    // this should append the deleted positions to the list
    m_psa.particleGroup(groupId)->releaseParticlesGetPositions(volume, m_remeberedParticles);
    assert(m_remeberedParticles.size() >= nbParticlesBefore);

    return static_cast<unsigned int>(m_remeberedParticles.size()) - nbParticlesBefore;
}

unsigned int ParticleCollision::releaseForgetParticles(int groupId, const AxisAlignedBoundingBox & volume)
{
    return m_psa.particleGroup(groupId)->releaseParticles(volume);
}

unsigned int ParticleCollision::createFromRemembered(const std::string & elementName)
{
    particleGroup(elementName)->createParticles(m_remeberedParticles);
    return static_cast<unsigned int>(m_remeberedParticles.size());
}

int ParticleCollision::particleGroupId(const std::string & elementName)
{
    const auto it = m_particleGroupIds.find(elementName);
    if (it != m_particleGroupIds.end())
        return it->second;

    int newId = m_psa.createParticleGroup(false, elementName);
    m_particleGroupIds.emplace(elementName, newId);
    return newId;
}

ParticleGroup * ParticleCollision::particleGroup(const std::string & elementName)
{
    return m_psa.particleGroup(particleGroupId(elementName));
}
