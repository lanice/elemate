#include "watertile.h"

#include <glow/Shader.h>
#include <glow/Program.h>
#include <glowutils/File.h>

WaterTile::WaterTile(Terrain & terrain, const TileID & tileID)
: TerrainTile(terrain, tileID)
{
}

void WaterTile::bind(const glowutils::Camera & camera)
{
    TerrainTile::bind(camera);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
}

void WaterTile::unbind()
{
    glDisable(GL_BLEND);
}

void WaterTile::initializeProgram()
{
    glow::ref_ptr<glow::Shader> vertex = glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/terrain_water.vert");
    glow::ref_ptr<glow::Shader> fragment = glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/terrain_water.frag");
    glow::ref_ptr<glow::Shader> phongLightingFrag = glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/phongLighting.frag");

    m_program = new glow::Program();
    m_program->attach(vertex, fragment, phongLightingFrag);
}
