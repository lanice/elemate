#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#include <osg/ref_ptr>

namespace osg {
    class Matrixf;
    class Uniform;
    class StateSet;
}
namespace physx {
    class PxPhysics;
    class PxMaterial;
}
class PhysicsWrapper;

struct Elements {
    /** Call this function once before calling for the elements. */
    static void initialize(physx::PxPhysics & physxSdk);

    static physx::PxMaterial * pxMaterial(const std::string & physxMaterial);
    static const osg::Matrixf * shadingMatrix(const std::string & shadingMaterial);

    /** @return osg matrix uniform with name (s_materialUniformPrefix + shadingMaterial) containing the material defition for shading */
    static osg::Uniform * getUniform(const std::string & shadingMaterial);
    /** adds osg matrix uniforms with name (s_materialUniformPrefix + shadingMaterial) for all given material names
        @ return false, if at least one material is not defined. */
    static bool addUniforms(const std::vector<std::string> & shadingMaterials, osg::StateSet & stateSet);
    /** adds osg matrix uniforms with name (s_materialUniformPrefix + shadingMaterial) for all materials */
    static void addAllUniforms(osg::StateSet & stateSet);

    static const std::string s_materialUniformPrefix;

private:
    static bool s_isInitialized;

    static void createUniforms();

    static std::unordered_map<std::string, physx::PxMaterial*>	        s_pxMaterials;
    static std::unordered_map<std::string, osg::Matrixf>                s_shadingMatices;
    static std::unordered_map<std::string, osg::ref_ptr<osg::Uniform>>  s_shadingMatrixUniforms;
};
