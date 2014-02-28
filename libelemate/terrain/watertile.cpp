#include "watertile.h"

#include <glow/Shader.h>
#include <glow/Program.h>
#include <glow/Texture.h>
#include <glowutils/global.h>

#include "terrain.h"
#include "world.h"
#include "texturemanager.h"

WaterTile::WaterTile(Terrain & terrain, const TileID & tileID)
: PhysicalTile(terrain, tileID, {"water"})
{
}

void WaterTile::bind(const CameraEx & camera)
{
    PhysicalTile::bind(camera);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
}

void WaterTile::unbind()
{
    glDisable(GL_BLEND);

    PhysicalTile::unbind();
}

void WaterTile::initializeProgram()
{
    m_program = new glow::Program();
    m_program->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/terrain_water.vert"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/terrain_water.frag"),
        World::instance()->sharedShader(GL_FRAGMENT_SHADER, "shader/utils/phongLighting.frag"));

    std::string baseTileName = generateName(TileID(TerrainLevel::BaseLevel, m_tileID.x, m_tileID.z));
    m_program->setUniform("baseHeightField", TextureManager::getTextureUnit(baseTileName, "values"));

    PhysicalTile::initializeProgram();
}

uint8_t WaterTile::elementIndexAt(unsigned int /*row*/, unsigned int /*column*/) const
{
    return 0u;  // currently water only, no data for different elements
}

uint8_t WaterTile::elementIndex(const std::string & elementName) const
{
    if (elementName != "water")
        glow::warning("Trying to fetch an element other than water from water terrain level.");
    assert(elementName == "water");
    return 0u;
}

void WaterTile::setElement(unsigned int /*row*/, unsigned int /*column*/, uint8_t /*elementIndex*/)
{
    glow::warning("setting element type on WaterTile is not supported.");
    assert(false);
}

