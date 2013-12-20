#include "elements.h"

#include <cassert>
#include <iostream>

#include <glow/Program.h>

#include <PxPhysics.h>

bool Elements::s_isInitialized = false;
std::unordered_map<std::string, physx::PxMaterial*>	Elements::s_pxMaterials;
std::unordered_map<std::string, glm::mat4>          Elements::s_shadingMatices;

const std::string Elements::s_materialUniformPrefix = "material_";

void Elements::initialize(physx::PxPhysics & physxSdk)
{
    assert(!s_isInitialized);

    s_pxMaterials.emplace("default", physxSdk.createMaterial(0.5f, 0.5f, 0.1f));

    s_shadingMatices.emplace("default", glm::mat4(
        0.0f, 0.0f, 0.0f, 1.0f,    //ambient
        1.0f, 1.0f, 1.0f, 1.0f,    //diffuse
        1.0f, 1.0f, 1.0f, 1.0f,    //specular
        0.0f, 0.0f, 0.0f, 0.0f));  //emission

    s_shadingMatices.emplace("bedrock", glm::mat4(
        0.0f, 0.0f, 0.0f, 1.0f,    //ambient
        0.52f, 0.49f, 0.44f, 1.0f,    //diffuse
        0.52f, 0.49f, 0.44f, 1.0f,    //specular
        0.0f, 0.0f, 0.0f, 0.0f));  //emission

    s_shadingMatices.emplace("dirt", glm::mat4(
        0.0f, 0.0f, 0.0f, 1.0f,    //ambient
        0.66f, 0.55f, 0.4f, 1.0f,    //diffuse
        0.66f, 0.55f, 0.4f, 1.0f,    //specular
        0.0f, 0.0f, 0.0f, 0.0f));  //emission

    s_shadingMatices.emplace("grassland", glm::mat4(
        0.0f, 0.0f, 0.0f, 1.0f,    //ambient
        0.2f, 0.7f, 0.3f, 1.0f,    //diffuse
        0.2f, 0.7f, 0.3f, 1.0f,    //specular
        0.0f, 0.0f, 0.0f, 0.0f));  //emission

    s_shadingMatices.emplace("water", glm::mat4(
        0.0f, 0.0f, 0.0f, 1.0f,    //ambient
        0.3f, 0.6f, 0.9f, 1.0f,    //diffuse
        0.5f, 0.7f, 0.9f, 1.0f,    //specular
        0.0f, 0.0f, 0.0f, 0.0f));  //emission

    for (const auto & pair : s_pxMaterials)
        assert(pair.second);

    s_isInitialized = true;
}

void Elements::setAllUniforms(glow::Program & program)
{
    for (const auto & pair : s_shadingMatices)
        program.setUniform(s_materialUniformPrefix + pair.first, pair.second);
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
