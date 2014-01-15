#pragma once

#include <unordered_map>
#include <list>
#include <string>

#include <glm/glm.hpp>

namespace glow {
    class Program;
}

namespace physx {
    class PxPhysics;
    class PxMaterial;
}
class PhysicsWrapper;
class EmitterDescriptionData;

struct Elements {
    Elements() = delete;

    /** Call this function once before calling for the elements. */
    static void initialize(physx::PxPhysics & physxSdk);
    static void clear();

    static void setAllUniforms(glow::Program & program);

    static physx::PxMaterial * pxMaterial(const std::string & physxMaterial);
    static const glm::mat4 * shadingMatrix(const std::string & shadingMaterial);
    static EmitterDescriptionData* emitterDescription(const std::string & descriptionData);
    static std::string sound(const std::string & sound);

    static const std::list<std::string>& availableElements();

    static const std::string s_materialUniformPrefix;

private:
    static bool s_isInitialized;

    static std::list<std::string>                                    s_availableElements;
    static std::unordered_map<std::string, physx::PxMaterial*>	     s_pxMaterials;
    static std::unordered_map<std::string, glm::mat4>                s_shadingMatrices;
    static std::unordered_map<std::string, std::string>              s_soundFiles;
    static std::unordered_map<std::string, EmitterDescriptionData*>  s_emitterDescriptionData;
};
