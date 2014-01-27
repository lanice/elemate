#include "basetile.h"

#include <glow/Program.h>
#include <glow/Buffer.h>
#include <glowutils/File.h>

#include "terrain.h"
#include "imagereader.h"
#include "world.h"

BaseTile::BaseTile(Terrain & terrain, const TileID & tileID, const std::initializer_list<std::string> & elementNames)
: TerrainTile(terrain, tileID, elementNames)
, m_terrainTypeTex(nullptr)
, m_terrainTypeBuffer(nullptr)
, m_terrainTypeData(nullptr)
{
}

BaseTile::~BaseTile()
{
    delete m_terrainTypeData;
}

void BaseTile::bind(const CameraEx & camera)
{
    TerrainTile::bind(camera);

    assert(m_terrainTypeTex);
    m_terrainTypeTex->bind(GL_TEXTURE1);

    for (TextureTuple & tex : m_textures)
        std::get<1>(tex)->bind(GL_TEXTURE0 + std::get<2>(tex));
}

void BaseTile::unbind()
{
    for (TextureTuple & tex : m_textures)
        std::get<1>(tex)->unbind(GL_TEXTURE0 + std::get<2>(tex));

    TerrainTile::unbind();
}

void BaseTile::initialize()
{
    TerrainTile::initialize();

    createTerrainTypeTexture();

    loadInitTexture("bedrock", 2);
    loadInitTexture("sand", 3);     // http://opengameart.org/content/50-free-textures
}

void BaseTile::initializeProgram()
{
    m_program = new glow::Program();
    m_program->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/terrain_base.vert"),
        glowutils::createShaderFromFile(GL_GEOMETRY_SHADER, "shader/terrain_base.geo"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/terrain_base.frag"),
        World::instance()->sharedShader(GL_FRAGMENT_SHADER, "shader/phongLighting.frag"));

    m_program->setUniform("terrainTypeID", 1);
    for (TextureTuple & tex : m_textures)
        m_program->setUniform(std::get<0>(tex), std::get<2>(tex));

    m_program->setUniform("modelTransform", m_transform);

    TerrainTile::initializeProgram();
}

void BaseTile::createTerrainTypeTexture()
{
    assert(m_terrainTypeData);

    m_terrainTypeBuffer = new glow::Buffer(GL_TEXTURE_BUFFER);
    m_terrainTypeBuffer->setData(*m_terrainTypeData, GL_DYNAMIC_DRAW);

    m_terrainTypeTex = new glow::Texture(GL_TEXTURE_BUFFER);
    m_terrainTypeTex->bind();
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R8UI, m_terrainTypeBuffer->id());
    m_terrainTypeTex->unbind();
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
    texture->unbind();

    m_textures.push_back(TextureTuple(elementName + "Sampler", texture, textureSlot));
}

void BaseTile::updateGlBuffers()
{
    m_terrainTypeBuffer->setData(*m_terrainTypeData, GL_DYNAMIC_DRAW);
    TerrainTile::updateGlBuffers();
}

uint8_t BaseTile::elementIndexAt(unsigned int row, unsigned int column) const
{
    return m_terrainTypeData->at(column + row * m_terrain.settings.columns);
}
