#include "elements.h"

#include <cassert>

#include <osg/Matrixf>
#include <osg/Uniform>
#include <osg/StateSet>

#include <PxPhysics.h>

bool Elements::s_isInitialized = false;
std::unordered_map<std::string, physx::PxMaterial*>	        Elements::s_pxMaterials;
std::unordered_map<std::string, osg::Matrixf>               Elements::s_shadingMatices;
std::unordered_map<std::string, osg::ref_ptr<osg::Uniform>> Elements::s_shadingMatrixUniforms;

const std::string Elements::s_materialUniformPrefix = "material_";

void Elements::initialize(physx::PxPhysics & physxSdk)
{
    assert(!s_isInitialized);

    s_pxMaterials.emplace("default", physxSdk.createMaterial(0.5f, 0.5f, 0.1f));
    s_shadingMatices.emplace("default", osg::Matrixf(
        0.0f, 0.0f, 0.0f, 1.0f,    //ambient
        1.0f, 1.0f, 1.0f, 1.0f,    //diffuse
        1.0f, 1.0f, 1.0f, 1.0f,    //specular
        0.0f, 0.0f, 0.0f, 0.0f));  //emission

    s_shadingMatices.emplace("bedrock", osg::Matrixf(
        0.0f, 0.0f, 0.0f, 1.0f,    //ambient
        0.52f, 0.49f, 0.44f, 1.0f,    //diffuse
        0.52f, 0.49f, 0.44f, 1.0f,    //specular
        0.0f, 0.0f, 0.0f, 0.0f));  //emission

    s_shadingMatices.emplace("dirt", osg::Matrixf(
        0.0f, 0.0f, 0.0f, 1.0f,    //ambient
        0.66f, 0.55f, 0.4f, 1.0f,    //diffuse
        0.66f, 0.55f, 0.4f, 1.0f,    //specular
        0.0f, 0.0f, 0.0f, 0.0f));  //emission

    s_shadingMatices.emplace("grassland", osg::Matrixf(
        0.0f, 0.0f, 0.0f, 1.0f,    //ambient
        0.2f, 0.7f, 0.3f, 1.0f,    //diffuse
        0.2f, 0.7f, 0.3f, 1.0f,    //specular
        0.0f, 0.0f, 0.0f, 0.0f));  //emission

    s_shadingMatices.emplace("water", osg::Matrixf(
        0.0f, 0.0f, 0.0f, 1.0f,    //ambient
        0.3f, 0.6f, 0.9f, 1.0f,    //diffuse
        0.5f, 0.7f, 0.9f, 1.0f,    //specular
        0.0f, 0.0f, 0.0f, 0.0f));  //emission

    for (const auto & pair : s_pxMaterials)
        assert(pair.second);

    createUniforms();

    s_isInitialized = true;
}

void Elements::createUniforms()
{
    assert(!s_isInitialized);

    for (const auto & pair : s_shadingMatices)
    {
        const std::string & name = pair.first;
        const osg::Matrixf & shadingMaterial = pair.second;
        s_shadingMatrixUniforms.emplace(name, new osg::Uniform((s_materialUniformPrefix + name).c_str(), shadingMaterial));
    }
}

physx::PxMaterial * Elements::pxMaterial(const std::string & physxMaterial)
{
    assert(s_isInitialized);

    const auto & it = s_pxMaterials.find(physxMaterial);
    // for debugging: make sure to address only valid elements
    bool found = it != s_pxMaterials.end();
    assert(found);
    if (!found)
        return nullptr;

    return it->second;
}

const osg::Matrixf * Elements::shadingMatrix(const std::string & shadingMaterial)
{
    assert(s_isInitialized);

    const auto & it = s_shadingMatices.find(shadingMaterial);
    // for debugging: make sure to address only valid elements
    bool found = it != s_shadingMatices.end();
    assert(found);
    if (!found)
        return nullptr;

    return &it->second;
}

osg::Uniform * Elements::getUniform(const std::string & shadingMaterial)
{
    assert(s_isInitialized);

    const auto & it = s_shadingMatrixUniforms.find(shadingMaterial);
    // for debugging: make sure to address only valid elements
    bool found = it != s_shadingMatrixUniforms.end();
    assert(found);
    if (!found)
        return nullptr;

    return it->second.get();
}

bool Elements::addUniforms(const std::vector<std::string> & shadingMaterials, osg::StateSet & stateSet)
{
    bool foundAll = true;
    for (const std::string & materialName : shadingMaterials) {
        auto * uniform = getUniform(materialName);
        assert(uniform);
        foundAll = (uniform != nullptr) && foundAll;
        if (uniform)
            stateSet.addUniform(uniform);
    }
    return foundAll;
}

void Elements::addAllUniforms(osg::StateSet & stateSet)
{
    for (const auto & pair : s_shadingMatrixUniforms) {
        stateSet.addUniform(pair.second.get());
    }
}
