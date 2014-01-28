#include "elements.h"

#include <cassert>

#include <glow/Program.h>

#include "pxcompilerfix.h"
#include <PxPhysics.h>
#include <PxMaterial.h>

#include "terrain/terrainsettings.h"

bool Elements::s_isInitialized = false;

std::unordered_map<std::string, physx::PxMaterial*>	Elements::s_pxMaterials;
std::unordered_map<std::string, glm::mat4>          Elements::s_shadingMatrices;

const std::string Elements::s_elementUniformPrefix = "element_";

void Elements::initialize()
{
    assert(!s_isInitialized);

    s_pxMaterials.emplace("default", PxGetPhysics().createMaterial(0.5f, 0.5f, 0.1f));

    s_shadingMatrices.emplace("water", glm::mat4(
        0.0f, 0.0f, 0.0f, 1.0f,    //ambient
        0.3f, 0.6f, 0.9f, 1.0f,    //diffuse
        0.5f, 0.7f, 0.9f, 1.0f,    //specular
        0.0f, 0.0f, 0.0f, 0.0f));  //emission

    s_shadingMatrices.emplace("default", glm::mat4(
        0.0f, 0.0f, 0.0f, 1.0f,    //ambient
        1.0f, 1.0f, 1.0f, 1.0f,    //diffuse
        1.0f, 1.0f, 1.0f, 1.0f,    //specular
        0.0f, 0.0f, 0.0f, 0.0f));  //emission

    s_shadingMatrices.emplace("bedrock", glm::mat4(
        0.0f, 0.0f, 0.0f, 1.0f,    //ambient
        0.52f, 0.49f, 0.44f, 1.0f,    //diffuse
        0.52f, 0.49f, 0.44f, 1.0f,    //specular
        0.0f, 0.0f, 0.0f, 0.0f));  //emission

    s_shadingMatrices.emplace("dirt", glm::mat4(
        0.0f, 0.0f, 0.0f, 1.0f,    //ambient
        0.66f, 0.55f, 0.4f, 1.0f,    //diffuse
        0.66f, 0.55f, 0.4f, 1.0f,    //specular
        0.0f, 0.0f, 0.0f, 0.0f));  //emission

    s_shadingMatrices.emplace("sand", glm::mat4(
        0.0f, 0.0f, 0.0f, 1.0f,    //ambient
        0.75f, 0.72f, 0.61f, 1.0f,    //diffuse
        0.75f, 0.72f, 0.61f, 1.0f,    //specular
        0.0f, 0.0f, 0.0f, 0.0f));  //emission

    s_shadingMatrices.emplace("grassland", glm::mat4(
        0.0f, 0.0f, 0.0f, 1.0f,    //ambient
        0.2f, 0.7f, 0.3f, 1.0f,    //diffuse
        0.2f, 0.7f, 0.3f, 1.0f,    //specular
        0.0f, 0.0f, 0.0f, 0.0f));  //emission

    for (const auto & pair : s_pxMaterials) {
        assert(pair.second);
        if (!pair.second)
            glow::warning("Elements::initialize could not create PhysX material: %;", pair.first);
    }

    initElementTerrainLevels();

    s_isInitialized = true;
}

void Elements::clear()
{
    for (auto mat : s_pxMaterials)
        mat.second->release();

    s_pxMaterials.clear();
    s_shadingMatrices.clear();
}

void Elements::setAllUniforms(glow::Program & program)
{
    for (const auto & pair : s_shadingMatrices)
        program.setUniform(s_elementUniformPrefix + pair.first, pair.second);
}

physx::PxMaterial * Elements::pxMaterial(const std::string & elementName)
{
    assert(s_isInitialized);

    const auto & it = s_pxMaterials.find(elementName);
    // for debugging: make sure to address only valid elements
    bool found = it != s_pxMaterials.end();
    if (!found) {
        glow::warning("Physx material undefined for element ""%;"". using default element instead.", elementName);
        return s_pxMaterials.at("default");
    }

    return it->second;
}
