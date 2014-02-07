#pragma once

namespace glowutils {
    class AxisAlignedBoundingBox;
}
class ParticleScriptAccess;

class ParticleCollision
{
public:
    ParticleCollision(ParticleScriptAccess & psa);

    /** check collision between the psa's particle group bounding boxes and call the scripts for further steps */
    void performCheck();

    static bool checkBoundingBoxCollision(const glowutils::AxisAlignedBoundingBox & box1, const glowutils::AxisAlignedBoundingBox & box2);

protected:
    ParticleScriptAccess & m_psa;

public:
    void operator=(ParticleCollision&) = delete;
};
