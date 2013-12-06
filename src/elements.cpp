#include "elements.h"

#include <cassert>

#include <PxPhysics.h>

bool Elements::s_isInitialized = false;
std::unordered_map<std::string, physx::PxMaterial*>	Elements::s_materials;

void Elements::initialize(physx::PxPhysics & physxSdk)
{
    assert(!s_isInitialized);

    s_materials.emplace("default", physxSdk.createMaterial(0.5f, 0.5f, 0.1f));

    for (const auto & pair : s_materials)
        assert(pair.second);

    s_isInitialized = true;
}

physx::PxMaterial * Elements::pxMaterial(const std::string & name)
{
    assert(s_isInitialized);

    const auto & it = s_materials.find(name);
    // for debugging: make sure to address only valid elements
    assert(it != s_materials.end());
    if (it == s_materials.end())
        return nullptr;

    return it->second;
}
