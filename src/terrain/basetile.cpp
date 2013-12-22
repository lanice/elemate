#include "basetile.h"

#include <glow/Shader.h>
#include <glow/Program.h>
#include <glowutils/File.h>

BaseTile::BaseTile(const TileID & tileID)
: TerrainTile(tileID)
{
}

void BaseTile::initializeProgram()
{
    glow::ref_ptr<glow::Shader> vertex = glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/terrain_base.vert");
    glow::ref_ptr<glow::Shader> fragment = glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/terrain_base.frag");
    glow::ref_ptr<glow::Shader> phongLightingFrag = glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/phongLighting.frag");

    m_program = new glow::Program();
    m_program->attach(vertex, fragment, phongLightingFrag);
}
