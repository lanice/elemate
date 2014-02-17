#pragma once

#include <list>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>

#include <glm/glm.hpp>

namespace glowutils {
    class AxisAlignedBoundingBox;
}
class ParticleScriptAccess;
class ParticleGroup;
class LuaWrapper;

class ParticleCollision
{
public:
    ParticleCollision(ParticleScriptAccess & psa);
    ~ParticleCollision();

    /** check collision between the psa's particle group bounding boxes and call the scripts for further steps */
    void performCheck();

    /** @return whether the input axis aligned bounding boxes intersect
      * @param intersectVolume will be set to the intersection volume, if the boxes intersect and the parameter is not set to nullptr */
    static bool checkBoundingBoxCollision(const glowutils::AxisAlignedBoundingBox & box1, const glowutils::AxisAlignedBoundingBox & box2, glowutils::AxisAlignedBoundingBox * intersectVolume = nullptr);

    void particleGroupDeleted(const std::string & elementName, int id);
    void clearParticleGroups();

protected:
    ParticleScriptAccess & m_psa;
    LuaWrapper * m_lua;

    /** Register my functions that can be called from lua.
      * If a relevant collision occurs, this class will call elementReaction() in lua. Lua functions called on this class
      * have effect in the context of the currently processed collision, that's why the registered functions shouldn't be
      * called from a lua state outside of this class. */
    void registerLuaFunctions();

    struct IntersectionBox {
        glm::vec3 llf;
        glm::vec3 urb;
    };


    void checkCollidedParticles(int leftGroup, int rightGroup, const glowutils::AxisAlignedBoundingBox & intersectVolume);
    std::pair<int, int> treeCheck(const glowutils::AxisAlignedBoundingBox & volume, const std::vector<glm::vec3> & leftHandPositions, const std::vector<glm::vec3> & rightHandPositions, int depth);
    /** this list contains particles released by the script, but which i will remember to create new particles at the same positions, if requested. */
    std::vector<glm::vec3> m_remeberedParticles;
    unsigned int forgetOldParticles();
    unsigned int releaseRemeberParticles(int groupId, const glowutils::AxisAlignedBoundingBox & volume);
    unsigned int releaseForgetParticles(int groupId, const glowutils::AxisAlignedBoundingBox & volume);
    unsigned int createFromRemembered(const std::string & elementName);

    /** reduce the point sets to positions that are inside the bounding box of the comparing set
      * @return true, if there are particles in a common sub box, false otherwise */
    static bool extractCommonPositionBox(const std::vector<glm::vec3> & leftHandPositions, const std::vector<glm::vec3> & rightHandPositions,
        const glowutils::AxisAlignedBoundingBox & leftBBox, const glowutils::AxisAlignedBoundingBox & rightBBox,
        std::vector<glm::vec3> & leftHandExtracted, std::vector<glm::vec3> & rightHandExtracted,
        glowutils::AxisAlignedBoundingBox & commonBBox);

    /** for now: maintain one particle group (id) for each element that results of an element reaction */
    std::unordered_map<std::string, int> m_particleGroupIds;
    /** get the group id for the element and create the group if needed */
    int particleGroupId(const std::string & elementName);
    ParticleGroup * particleGroup(const std::string & elementName);
    /** for graphical debugging: the current list of intersection volumes **/
    static std::list<IntersectionBox> debug_intersectionBoxes;
    friend class DebugStep;

public:
    void operator=(ParticleCollision&) = delete;
};
