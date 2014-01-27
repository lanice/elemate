#include "watertile.h"

#include <glow/Shader.h>
#include <glow/Program.h>
#include <glowutils/File.h>

#include "terrain.h"
#include "world.h"

WaterTile::WaterTile(Terrain & terrain, const TileID & tileID)
: TerrainTile(terrain, tileID, {"water"})
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

uint8_t WaterTile::elementIndexAt(unsigned int /*row*/, unsigned int /*column*/) const
{
    return 0u;  // currently water only, no data for different elements
}

uint8_t WaterTile::elementIndex(const std::string & elementName) const
{
    assert(elementName == "water");
    return 0u;
}

void WaterTile::setElement(unsigned int /*row*/, unsigned int /*column*/, uint8_t /*elementIndex*/)
{
    glow::warning("setting element type on WaterTile is not supported.");
    assert(false);
}

