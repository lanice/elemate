#include "basetile.h"

#include <glow/Shader.h>
#include <glow/Program.h>
#include <glowutils/File.h>

#include <PxMaterial.h>
#include <geometry/PxHeightFieldSample.h>
#include <geometry/PxHeightFieldDesc.h>

#include "terrain.h"
#include "elements.h"

BaseTile::BaseTile(Terrain & terrain, const TileID & tileID)
: TerrainTile(terrain, tileID)
, m_terrainTypeTex(nullptr)
, m_terrainTypeData(nullptr)
{
}

BaseTile::~BaseTile()
{
    delete m_terrainTypeData;
}

void BaseTile::bind(const glowutils::Camera & camera)
{
    if (!m_terrainTypeTex)
        createTerrainTypeTexture();

    glActiveTexture(GL_TEXTURE0 + 1);
    m_terrainTypeTex->bind();

    TerrainTile::bind(camera);
}

void BaseTile::unbind()
{
    TerrainTile::unbind();
}

void BaseTile::initializeProgram()
{
    glow::ref_ptr<glow::Shader> vertex = glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/terrain_base.vert");
    glow::ref_ptr<glow::Shader> fragment = glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/terrain_base.frag");
    glow::ref_ptr<glow::Shader> phongLightingFrag = glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/phongLighting.frag");

    m_program = new glow::Program();
    m_program->attach(vertex, fragment, phongLightingFrag);

    m_program->setUniform("terrainTypeID", 1);
}

using namespace physx;

void BaseTile::pxSamplesAndMaterials(PxHeightFieldSample * hfSamples, PxReal heightScale, PxMaterial ** &materials)
{
    materials = new PxMaterial*[1];

    materials[0] = Elements::pxMaterial("default");
    //materials[1] = Elements::pxMaterial("default");

    unsigned int numSamples = m_terrain->settings.rows * m_terrain->settings.columns;
    for (unsigned index = 0; index < numSamples; ++index) {
        hfSamples[index].materialIndex0 = hfSamples[index].materialIndex1
            //= m_terrainTypeData->at(index);
            = 0;    // no special px materials for now
        hfSamples[index].height = static_cast<PxI16>(m_heightField->at(index) * heightScale);
    }
}

void BaseTile::createTerrainTypeTexture()
{
    assert(m_terrainTypeData);

    m_terrainTypeTex = new glow::Texture(GL_TEXTURE_2D);
    m_terrainTypeTex->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_terrainTypeTex->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_terrainTypeTex->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_terrainTypeTex->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_terrainTypeTex->setParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    m_terrainTypeTex->image2D(0, GL_R8UI, m_terrain->settings.rows, m_terrain->settings.columns, 0, GL_RED_INTEGER,
        GL_UNSIGNED_BYTE, m_terrainTypeData->rawData());
}
