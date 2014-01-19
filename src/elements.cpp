#include "elements.h"

#include <cassert>

#include <glow/Program.h>

#include "pxcompilerfix.h"
#include <PxPhysics.h>
#include <PxMaterial.h>

#include "particleemitter.h"
#include "terrain/terrainsettings.h"

bool Elements::s_isInitialized = false;

std::list<std::string>                              Elements::s_availableElements;
std::unordered_map<std::string, physx::PxMaterial*>	Elements::s_pxMaterials;
std::unordered_map<std::string, glm::mat4>          Elements::s_shadingMatrices;
std::unordered_map<std::string, std::string>        Elements::s_soundFiles;
std::unordered_map<std::string, EmitterDescriptionData*> Elements::s_emitterDescriptionData;

const std::string Elements::s_materialUniformPrefix = "material_";

void Elements::initialize(physx::PxPhysics & physxSdk)
{
    assert(!s_isInitialized);

    s_pxMaterials.emplace("default", physxSdk.createMaterial(0.5f, 0.5f, 0.1f));

    s_availableElements.push_back("water");
    s_emitterDescriptionData.emplace("water", new EmitterDescriptionData());
    s_soundFiles.emplace("water", "data/sounds/fountain_loop.wav");
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

    initMaterialTerrainLevels();

    s_isInitialized = true;
}

const std::list<std::string>& Elements::availableElements(){
    return s_availableElements;
}

void Elements::clear()
{
    for (auto mat : s_pxMaterials)
        mat.second->release();

    for (auto mat : s_emitterDescriptionData)
        delete mat.second;

    s_pxMaterials.clear();
    s_emitterDescriptionData.clear();
    s_shadingMatrices.clear();
    s_soundFiles.clear();
}

void Elements::setAllUniforms(glow::Program & program)
{
    for (const auto & pair : s_shadingMatrices)
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

EmitterDescriptionData* Elements::emitterDescription(const std::string & descriptionData){
    assert(s_isInitialized);

    const auto & it = s_emitterDescriptionData.find(descriptionData);
    // for debugging: make sure to address only valid elements
    bool found = it != s_emitterDescriptionData.end();
    assert(found);
    if (!found)
        return nullptr;

    return it->second;
}