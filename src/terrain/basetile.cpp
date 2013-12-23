#include "basetile.h"

#include <glow/Shader.h>
#include <glow/Program.h>
#include <glowutils/File.h>

#include <geometry/PxHeightFieldSample.h>

#include "terrain.h"

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
    assert(m_terrainTypeTex);

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

void BaseTile::createTerrainTypeTexture(const physx::PxHeightFieldSample * pxHeightFieldSamples)
{
    unsigned int numSamples = m_terrain->settings.rows * m_terrain->settings.columns;

    m_terrainTypeData = new glow::UByteArray();
    m_terrainTypeData->resize(numSamples);

    for (unsigned index = 0; index < numSamples; ++index) {
        m_terrainTypeData->at(index) = pxHeightFieldSamples[index].materialIndex0;
    }

    m_terrainTypeTex = new glow::Texture(GL_TEXTURE_2D);
    m_terrainTypeTex->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_terrainTypeTex->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_terrainTypeTex->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_terrainTypeTex->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_terrainTypeTex->setParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    m_terrainTypeTex->image2D(0, GL_R8UI, m_terrain->settings.rows, m_terrain->settings.columns, 0, GL_RED_INTEGER,
        GL_UNSIGNED_BYTE, m_terrainTypeData->rawData());
}
