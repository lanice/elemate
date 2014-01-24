#include "watertile.h"

#include <glow/Shader.h>
#include <glow/Program.h>
#include <glowutils/File.h>

#include <PxMaterial.h>
#include <geometry/PxHeightFieldSample.h>
#include <geometry/PxHeightFieldDesc.h>

#include "terrain.h"
#include "elements.h"
#include "world.h"

WaterTile::WaterTile(Terrain & terrain, const TileID & tileID)
: TerrainTile(terrain, tileID)
{
}

void WaterTile::bind(const CameraEx & camera)
{
    TerrainTile::bind(camera);

    assert(m_baseHeightTex);
    m_baseHeightTex->bind(GL_TEXTURE1);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
}

void WaterTile::unbind()
{
    glDisable(GL_BLEND);

    m_baseHeightTex->unbind(GL_TEXTURE1);

    TerrainTile::unbind();
}

void WaterTile::initializeProgram()
{
    m_program = new glow::Program();
    m_program->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/terrain_water.vert"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/terrain_water.frag"),
        World::instance()->sharedShader(GL_FRAGMENT_SHADER, "shader/phongLighting.frag"));

    m_program->setUniform("baseHeightField", 1);

    TerrainTile::initializeProgram();
}

using namespace physx;

void WaterTile::pxSamplesAndMaterials(PxHeightFieldSample * hfSamples, PxReal heightScale, PxMaterial ** &materials)
{
    materials = new PxMaterial*[1];

    // TODO: use water material..
    materials[0] = Elements::pxMaterial("default");

    unsigned int numSamples = m_terrain.settings.rows * m_terrain.settings.columns;
    for (unsigned index = 0; index < numSamples; ++index) {
        hfSamples[index].materialIndex0 = hfSamples[index].materialIndex1 = 0;
        hfSamples[index].height = static_cast<PxI16>(m_heightField->at(index) * heightScale);
    }
}

PxU8 WaterTile::pxMaterialIndexAt(unsigned int /*row*/, unsigned int /*column*/) const
{
    return 0u;
}
