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

protected:
    ParticleScriptAccess & m_psa;
    LuaWrapper * m_lua;

    struct IntersectionBox {
        IntersectionBox() = default;
        IntersectionBox(const glm::vec3 & llf, const glm::vec3 & urb);
        glm::vec3 llf;
        glm::vec3 urb;
        void operator=(const IntersectionBox&);
    };

    void treeCheck(const glowutils::AxisAlignedBoundingBox & volume, const std::vector<glm::vec3> & leftHandPositions, const std::vector<glm::vec3> & rightHandPositions, int depth);

    /** the two particle groups that are currently processed */
    ParticleGroup * m_currentLeftHand;
    ParticleGroup * m_currentRightHand;
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
