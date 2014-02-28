#include "basetile.h"

#include <algorithm>

#include <glow/Program.h>
#include <glow/Buffer.h>
#include <glow/Texture.h>
#include <glowutils/global.h>

#include "terrain.h"
#include "io/imagereader.h"
#include "world.h"
#include "texturemanager.h"


BaseTile::BaseTile(Terrain & terrain, const TileID & tileID, const std::initializer_list<std::string> & elementNames)
: PhysicalTile(terrain, tileID, elementNames)
, m_terrainTypeTex(nullptr)
, m_terrainTypeBuffer(nullptr)
{
    m_terrainTypeData.resize(samplesPerAxis * samplesPerAxis);
}

void BaseTile::bind(const CameraEx & camera)
{
    PhysicalTile::bind(camera);

    assert(m_terrainTypeTex);
}

void BaseTile::unbind()
{
    PhysicalTile::unbind();
}

void BaseTile::initialize()
{
    PhysicalTile::initialize();

    createTerrainTypeTexture();

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

    PhysicalTile::initializeProgram();
}

void BaseTile::createTerrainTypeTexture()
{
    m_terrainTypeBuffer = new glow::Buffer(GL_TEXTURE_BUFFER);
    m_terrainTypeBuffer->setData(m_terrainTypeData, GL_DYNAMIC_DRAW);

    m_terrainTypeTex = new glow::Texture(GL_TEXTURE_BUFFER);
    m_terrainTypeTex->bind();
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R8UI, m_terrainTypeBuffer->id());
    m_terrainTypeTex->unbind();

    m_terrainTypeTex->bindActive(GL_TEXTURE0 + TextureManager::reserveTextureUnit(tileName, "terrainType"));
    glActiveTexture(GL_TEXTURE0);
    CheckGLError();
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

void BaseTile::updateBuffers()
{
    assert(m_updateRangeMinMaxIndex.x < m_updateRangeMinMaxIndex.y);

    uint8_t * bufferDest = reinterpret_cast<uint8_t*>(m_terrainTypeBuffer->mapRange(
        m_updateRangeMinMaxIndex.x,
        m_updateRangeMinMaxIndex.y - m_updateRangeMinMaxIndex.x,
        GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT));
    assert(bufferDest);

    unsigned int indexOffset = m_updateRangeMinMaxIndex.x;

    for (const UpdateRange & range : m_bufferUpdateList) {
        assert(indexOffset <= range.startIndex);
        assert(range.startIndex - indexOffset >= 0);
        assert(range.startIndex - indexOffset + range.nbElements < m_terrainTypeData.size());
        memcpy(bufferDest + (range.startIndex - indexOffset),
            reinterpret_cast<uint8_t*>(m_terrainTypeData.data()) + range.startIndex,
            range.nbElements);
    }

    m_terrainTypeBuffer->unmap();

    PhysicalTile::updateBuffers();
}

uint8_t BaseTile::elementIndexAt(unsigned int row, unsigned int column) const
{
    return m_terrainTypeData.at(column + row * samplesPerAxis);
}

uint8_t BaseTile::elementIndex(const std::string & elementName) const
{
    size_t index = std::find(m_elementNames.cbegin(), m_elementNames.cend(), elementName) - m_elementNames.cbegin();
    assert(index < m_elementNames.size());
    return static_cast<uint8_t>(index);
}

void BaseTile::setElement(unsigned int row, unsigned int column, uint8_t elementIndex)
{
    assert(elementIndex < m_elementNames.size());
    assert(row < samplesPerAxis && column < samplesPerAxis);
    m_terrainTypeData.at(column + row * samplesPerAxis) = elementIndex;
}
