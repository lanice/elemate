#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace glow {
    class Program;
}

namespace physx {
    class PxPhysics;
    class PxMaterial;
}
class PhysicsWrapper;

struct Elements {
    /** Call this function once before calling for the elements. */
    static void initialize(physx::PxPhysics & physxSdk);

    static void setAllUniforms(glow::Program & program);

    static physx::PxMaterial * pxMaterial(const std::string & physxMaterial);
    static const glm::mat4 * shadingMatrix(const std::string & shadingMaterial);

    static const std::string s_materialUniformPrefix;

private:
    static bool s_isInitialized;

    static std::unordered_map<std::string, physx::PxMaterial*>	     s_pxMaterials;
    static std::unordered_map<std::string, glm::mat4>                s_shadingMatices;
};
