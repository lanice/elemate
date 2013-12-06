#pragma once

#include <unordered_map>
#include <string>

namespace physx {
    class PxPhysics;
    class PxMaterial;
}
class PhysicsWrapper;

struct Elements {
    /** Call this function once before calling for the elements. */
    static void initialize(physx::PxPhysics & physxSdk);

    static physx::PxMaterial * pxMaterial(const std::string & name);

private:
    static bool s_isInitialized;

    static std::unordered_map<std::string, physx::PxMaterial*>	s_materials;
};
