#pragma once

#include <unordered_map>
#include <list>
#include <string>

#include <glm/glm.hpp>

namespace glow {
    class Program;
}

namespace physx {
    class PxMaterial;
}

struct Elements {
    Elements() = delete;

    /** Call this function once before calling for the elements. */
    static void initialize();
    static void clear();

    static void setAllUniforms(glow::Program & program);

    static physx::PxMaterial * pxMaterial(const std::string & physxMaterial);

    static const std::string s_elementUniformPrefix;

private:
    static bool s_isInitialized;

    static std::unordered_map<std::string, physx::PxMaterial*>	     s_pxMaterials;
    static std::unordered_map<std::string, glm::mat4>                s_shadingMatrices;
};
