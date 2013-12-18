#pragma once

#include <unordered_map>
#include <string>
#include <vector>


namespace physx {
    class PxPhysics;
    class PxMaterial;
}
class PhysicsWrapper;

struct Elements {
    /** Call this function once before calling for the elements. */
    static void initialize(physx::PxPhysics & physxSdk);

    static physx::PxMaterial * pxMaterial(const std::string & physxMaterial);
    //static const osg::Matrixf * shadingMatrix(const std::string & shadingMaterial);

    static const std::string s_materialUniformPrefix;

private:
    static bool s_isInitialized;

    static void createUniforms();

    static std::unordered_map<std::string, physx::PxMaterial*>	        s_pxMaterials;
    //static std::unordered_map<std::string, osg::Matrixf>                s_shadingMatices;
    //static std::unordered_map<std::string, osg::ref_ptr<osg::Uniform>>  s_shadingMatrixUniforms;
};
