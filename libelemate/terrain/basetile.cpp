#include "basetile.h"

#include <algorithm>

#include <glow/Program.h>
#include <glow/Buffer.h>
#include <glow/Texture.h>
#include <glowutils/global.h>
#include "utils/cameraex.h"

#include "terrain.h"
#include "io/imagereader.h"
#include "world.h"
#include "texturemanager.h"
#include "elements.h"


BaseTile::BaseTile(Terrain & terrain, const TileID & tileID, const std::initializer_list<std::string> & elementNames)
: PhysicalTile(terrain, tileID, elementNames)
{
}

void BaseTile::bind(const CameraEx & camera)
{
    PhysicalTile::bind(camera);

    assert(m_terrainTypeTex);

    if (!m_program)
        initializeProgram();

    assert(m_program);

    m_program->use();
    m_program->setUniform("cameraposition", camera.eye());
    glm::mat4 modelView = camera.view() * m_transform;
    m_program->setUniform("modelView", modelView);
    glm::mat4 modelViewProjection = camera.viewProjectionEx() * m_transform;
    m_program->setUniform("modelViewProjection", modelViewProjection);
    m_program->setUniform("znear", camera.zNearEx());
    m_program->setUniform("zfar", camera.zFarEx());
    m_terrain.setDrawGridOffsetUniform(*m_program, camera.eye());
    m_program->setUniform("heightField", TextureManager::getTextureUnit(tileName, "values"));
    std::string temperatureTileName = generateName(TileID(TerrainLevel::TemperatureLevel, m_tileID.x, m_tileID.z));
    m_program->setUniform("temperatures", TextureManager::getTextureUnit(temperatureTileName, "values"));
    m_program->setUniform("drawHeatMap", m_drawHeatMap);

    World::instance()->setUpLighting(*m_program);
}

void BaseTile::unbind()
{
    m_program->release();

    PhysicalTile::unbind();
}

void BaseTile::initialize()
{
    PhysicalTile::initialize();

    loadInitTexture("bedrock", TextureManager::reserveTextureUnit(tileName, "bedrock"));
    loadInitTexture("sand", TextureManager::reserveTextureUnit(tileName, "sand"));     // http://opengameart.org/content/50-free-textures
    loadInitTexture("grassland", TextureManager::reserveTextureUnit(tileName, "grassland"));
}

void BaseTile::initializeProgram()
{
    m_program = new glow::Program();
    m_program->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/terrain_base.vert"),
        glowutils::createShaderFromFile(GL_GEOMETRY_SHADER, "shader/terrain_base.geo"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/terrain_base.frag"),
        World::instance()->sharedShader(GL_FRAGMENT_SHADER, "shader/utils/phongLighting.frag"));

    m_program->setUniform("terrainTypeID", TextureManager::getTextureUnit(tileName, "terrainType"));
    for (TextureTuple & tex : m_textures)
        m_program->setUniform(std::get<0>(tex), std::get<2>(tex));

    m_program->setUniform("modelTransform", m_transform);

    m_program->setUniform("modelTransform", m_transform);
    m_program->setUniform("tileSamplesPerAxis", int(samplesPerAxis));

    Elements::setAllUniforms(*m_program);
}

void BaseTile::loadInitTexture(const std::string & elementName, int textureSlot)
{
    glow::ref_ptr<glow::Texture> texture = new glow::Texture(GL_TEXTURE_2D);
    texture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    texture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    texture->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    texture->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

    RawImage image("data/textures/" + elementName + ".raw", 1024, 1024);

    texture->bind();
    texture->storage2D(8, GL_RGB8, image.width(), image.height());
    CheckGLError();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width(), image.height(), GL_RGB, GL_UNSIGNED_BYTE, image.rawData());
    CheckGLError();
    glGenerateMipmap(GL_TEXTURE_2D);
    CheckGLError();

    texture->bindActive(GL_TEXTURE0 + textureSlot);
    glActiveTexture(GL_TEXTURE0);
    CheckGLError();

    m_textures.push_back(TextureTuple(elementName + "Sampler", texture, textureSlot));
}

uint8_t BaseTile::elementIndexAt(unsigned int tileValueIndex) const
{
    assert(tileValueIndex < samplesPerAxis * samplesPerAxis);
    return m_terrainTypeData.at(tileValueIndex);
}

void BaseTile::setElement(unsigned int tileValueIndex, uint8_t elementIndex)
{
    assert(elementIndex < m_elementNames.size());
    assert(tileValueIndex < samplesPerAxis * samplesPerAxis);
    m_terrainTypeData.at(tileValueIndex) = elementIndex;
}
