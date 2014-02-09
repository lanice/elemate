#pragma once

#include <list>

#include <glm/glm.hpp>

namespace glowutils {
    class AxisAlignedBoundingBox;
}
class ParticleScriptAccess;
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

protected:
    ParticleScriptAccess & m_psa;
    LuaWrapper * m_lua;

    struct IntersectionBox {
        IntersectionBox(const glm::vec3 & llf, const glm::vec3 & urb);
        glm::vec3 llf;
        glm::vec3 urb;
        void operator=(const IntersectionBox&);
    };
    // for graphical debugging: the current list of intersection volumes
    static std::list<IntersectionBox> debug_intersectionBoxes;
    friend class DebugStep;

public:
    void operator=(ParticleCollision&) = delete;
};
