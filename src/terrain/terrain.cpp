#include "terrain.h"

#include <limits>
#include <algorithm>

#include <glow/VertexArrayObject.h>
#include <glow/VertexAttributeBinding.h>
#include <glow/Buffer.h>
#include <glow/Program.h>
#include <glowutils/Camera.h>
#include <glowutils/File.h>

#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "terraintile.h"

Terrain::Terrain(const TerrainSettings & settings)
: settings(settings)
, m_vao(nullptr)
, m_indexBuffer(nullptr)
, m_vbo(nullptr)
, m_vertices(nullptr)
, m_indices(nullptr)
, m_lightMapProgram(nullptr)
{
}

Terrain::~Terrain()
{
    m_vertices->clear();
    delete m_vertices;
    m_indices->clear();
    delete m_indices;
}

const GLuint restartIndex = std::numeric_limits<GLuint>::max();

void Terrain::draw(const glowutils::Camera & camera)
{
    // we probably don't want to draw an empty terrain
    assert(m_tiles.size() > 0);

    if (!m_vao)
        initialize();

    assert(m_vao);
    assert(m_indexBuffer);
    assert(m_vbo);

    m_vao->bind();

    glEnable(GL_PRIMITIVE_RESTART);

    for (auto & pair : m_tiles) {
        pair.second->bind(camera);
        glPrimitiveRestartIndex(restartIndex);
        m_vao->drawElements(GL_TRIANGLE_STRIP, static_cast<GLsizei>(m_indices->size()), GL_UNSIGNED_INT, nullptr);
        //m_vao->drawArrays(GL_TRIANGLE_STRIP_ADJACENCY, 0, m_vertices->size());

        pair.second->unbind();
    }

    m_vao->unbind();
}


//http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
glm::vec3 lightInvDir(0.0, 2.0, 3.0);

//// Compute the MVP matrix from the light's point of view
float zNear = -10;
float zFar = 20;
glm::mat4 depthProjectionMatrix = glm::ortho<float>(-10, 10, -10, 10, zNear, zFar);
glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
//glm::mat4 depthModelMatrix = glm::mat4(1.0);
//glm::mat4 depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;

void Terrain::drawLightMap(const glowutils::Camera & /*lightSource*/)
{
    // we probably don't want to draw an empty terrain
    assert(m_tiles.size() > 0);

    if (!m_vao)
        initialize();

    if (!m_lightMapProgram)
        initLightMapProgram();

    assert(m_vao);
    assert(m_indexBuffer);
    assert(m_vbo);

    m_vao->bind();

    glEnable(GL_PRIMITIVE_RESTART);

    m_lightMapProgram->use();

    m_lightMapProgram->setUniform("depthMVP", depthProjectionMatrix * depthViewMatrix * m_tiles.at(TileID(TerrainLevel::BaseLevel))->m_transform);
    m_lightMapProgram->setUniform("viewport", glm::ivec2(1024, 1024));
    m_lightMapProgram->setUniform("znear", zNear);
    m_lightMapProgram->setUniform("zfar", zFar);

    // TODO: generalize for more tiles...

    m_tiles.at(TileID(TerrainLevel::BaseLevel))->m_heightTex->bind(GL_TEXTURE0);
    m_tiles.at(TileID(TerrainLevel::WaterLevel))->m_heightTex->bind(GL_TEXTURE1);

    glPrimitiveRestartIndex(restartIndex);
    m_vao->drawElements(GL_TRIANGLE_STRIP, static_cast<GLsizei>(m_indices->size()), GL_UNSIGNED_INT, nullptr);

    m_tiles.at(TileID(TerrainLevel::BaseLevel))->m_heightTex->unbind(GL_TEXTURE0);
    m_tiles.at(TileID(TerrainLevel::WaterLevel))->m_heightTex->unbind(GL_TEXTURE1);

    m_lightMapProgram->release();

    m_vao->unbind();
}

glm::mat4 biasMatrix(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0
    );
//glm::mat4 depthBiasMVP = biasMatrix*depthMVP;

glow::FloatArray depthSamples;

void Terrain::drawShadowMapping(const glowutils::Camera & camera, const glowutils::Camera & /*lightSource*/)
{
    if (!m_shadowMappingProgram)
        initShadowMappingProgram();

    m_vao->bind();

    glEnable(GL_PRIMITIVE_RESTART);

    // TODO: generalize for more tiles...

    auto baseTile = m_tiles.at(TileID(TerrainLevel::BaseLevel));
    auto waterTile = m_tiles.at(TileID(TerrainLevel::WaterLevel));

    m_shadowMappingProgram->use();

    glm::mat4 depthBiasMVP = biasMatrix * depthProjectionMatrix * depthViewMatrix * baseTile->transform();

    m_shadowMappingProgram->setUniform("modelTransform", baseTile->transform());
    m_shadowMappingProgram->setUniform("modelViewProjection", camera.viewProjection() * m_tiles.at(TileID(TerrainLevel::BaseLevel))->m_transform);
    m_shadowMappingProgram->setUniform("lightSourceView", depthViewMatrix);
    m_shadowMappingProgram->setUniform("invView", camera.viewInverted());
    m_shadowMappingProgram->setUniform("viewport", camera.viewport());
    m_shadowMappingProgram->setUniform("znear", camera.zNear());
    m_shadowMappingProgram->setUniform("zfar", camera.zFar());
    m_shadowMappingProgram->setUniform("depthBiasMVP", depthBiasMVP);

    baseTile->m_heightTex->bind(GL_TEXTURE1);
    waterTile->m_heightTex->bind(GL_TEXTURE2);

    glPrimitiveRestartIndex(restartIndex);
    m_vao->drawElements(GL_TRIANGLE_STRIP, static_cast<GLsizei>(m_indices->size()), GL_UNSIGNED_INT, nullptr);

    baseTile->m_heightTex->unbind(GL_TEXTURE1);
    waterTile->m_heightTex->unbind(GL_TEXTURE2);

    m_lightMapProgram->release();

    m_vao->unbind();
}

void Terrain::initialize()
{
    generateVertices();
    generateIndices();

    m_vao = new glow::VertexArrayObject();

    m_indexBuffer = new glow::Buffer(GL_ELEMENT_ARRAY_BUFFER);
    m_indexBuffer->setData(*m_indices, GL_STATIC_DRAW);

    m_vbo = new glow::Buffer(GL_ARRAY_BUFFER);
    m_vbo->setData(*m_vertices, GL_STATIC_DRAW);

    m_vao->bind();

    glow::VertexAttributeBinding * vertexBinding = m_vao->binding(0);
    vertexBinding->setAttribute(0);
    vertexBinding->setBuffer(m_vbo, 0, sizeof(glm::vec2));
    vertexBinding->setFormat(2, GL_FLOAT, GL_FALSE, 0);
    m_vao->enable(0);

    m_indexBuffer->bind();

    m_vao->unbind();
}

void Terrain::initLightMapProgram()
{
    m_lightMapProgram = new glow::Program();

    m_lightMapProgram->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER,   "shader/shadows/lightmap_terrain.vert"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/shadows/depth_util.frag"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/shadows/lightmap_terrain.frag"));

    m_lightMapProgram->setUniform("heightField0", 0);
    m_lightMapProgram->setUniform("heightField1", 1);

    m_lightMapProgram->setUniform("tileRowsColumns", glm::uvec2(settings.rows, settings.columns));
}

void Terrain::initShadowMappingProgram()
{
    for (int i = 0; i < 128; ++i)
        depthSamples.push_back(glm::linearRand(-1.0f, 1.0f));

    m_shadowMappingProgram = new glow::Program();

    m_shadowMappingProgram->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/shadows/shadow_terrain.vert"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/shadows/depth_util.frag"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/shadows/shadow_terrain.frag"));

    m_shadowMappingProgram->setUniform("lightMap", 0);
    m_shadowMappingProgram->setUniform("heightField0", 1);
    m_shadowMappingProgram->setUniform("heightField1", 2);
    m_shadowMappingProgram->setUniform("depthSamples", depthSamples);

    m_shadowMappingProgram->setUniform("tileRowsColumns", glm::uvec2(settings.rows, settings.columns));
}

void Terrain::generateVertices()
{
    unsigned int numSamples = settings.rows * settings.columns;

    m_vertices = new glow::Vec2Array();
    m_vertices->resize(numSamples);

    for (unsigned row = 0; row < settings.rows; ++row) {
        const unsigned rowOffset = row * settings.columns;
        for (unsigned column = 0; column < settings.columns; ++column) {
            unsigned int index = column + rowOffset;

            // simply use the row/column position as vertex position, scaling with the transform matrix
            m_vertices->at(index) = glm::vec2(row, column);
        }
    }
}

void Terrain::generateIndices()
{
    m_indices = new glow::UIntArray;

    // create a quad for all vertices, except for the last row and column (covered by the forelast)
    // see PxHeightFieldDesc::samples documentation: "...(nbRows - 1) * (nbColumns - 1) cells are actually used."
    unsigned numIndices = (settings.rows - 1) * ((settings.columns) * 2 + 1);
    m_indices->reserve(numIndices);
    for (unsigned int row = 0; row < settings.rows - 1; ++row) {
        const unsigned rowOffset = row * settings.columns;
        for (unsigned int column = 0; column < settings.columns; ++column) {
            // "origin" is the left front vertex in a terrain quad
            const unsigned int origin = column + rowOffset;

            m_indices->push_back(origin);
            m_indices->push_back(origin + settings.columns);
        }
        m_indices->push_back(restartIndex);
    }

    assert(m_indices->size() == numIndices);
}

void Terrain::registerTile(const TileID & tileID, TerrainTile & tile)
{
    assert(m_tiles.find(tileID) == m_tiles.end());

    m_tiles.emplace(tileID, std::shared_ptr<TerrainTile>(&tile));
}

const std::map<TileID, physx::PxRigidStatic*> Terrain::pxActorMap() const
{
    return m_pxActors;
}

void Terrain::setUpLighting(glow::Program & program) const
{
    static glm::vec4 lightambientglobal(0, 0, 0, 0);
    static glm::vec3 lightdir1(0.0, 6.5, 7.5);
    static glm::vec3 lightdir2(0.0, -8.0, 7.5);

    static glm::mat4 lightMat1;
    lightMat1[0] = glm::vec4(0.0, 0.0, 0.0, 1.0);        //ambient
    lightMat1[1] = glm::vec4(0.2, 0.2, 0.2, 1.0);        //diffuse
    lightMat1[2] = glm::vec4(0.7, 0.7, 0.5, 1.0);        //specular
    lightMat1[3] = glm::vec4(0.002, 0.002, 0.0004, 1.4); //attenuation1, attenuation2, attenuation3, shininess

    static glm::mat4 lightMat2;
    lightMat2[0] = glm::vec4(0.0, 0.0, 0.0, 1.0);        //ambient
    lightMat2[1] = glm::vec4(0.1, 0.1, 0.1, 1.0);        //diffuse
    lightMat2[2] = glm::vec4(0.1, 0.1, 0.1, 1.0);        //specular
    lightMat2[3] = glm::vec4(0.002, 0.002, 0.0004, 1.4); //attenuation1, attenuation2, attenuation3, shininess

    program.setUniform("lightambientglobal", lightambientglobal);
    program.setUniform("lightdir1", lightdir1);
    program.setUniform("lightdir2", lightdir2);
    program.setUniform("light1", lightMat1);
    program.setUniform("light2", lightMat2);
}

float Terrain::heightAt(float x, float z) const
{
    float height = std::numeric_limits<float>::lowest();
    for (TerrainLevel level : TerrainLevels) {
        height = std::max(
            height,
            heightAt(x, z, level));
    }
    return height;
}

float Terrain::heightAt(float x, float z, TerrainLevel level) const
{
    std::shared_ptr<TerrainTile> tile = nullptr;
    float normX = 0.0f;
    float normZ = 0.0f;
    TileID tileID;
    if (!normalizePosition(x, z, tileID, normX, normZ))
        return 0.0f;

    tileID.level = level;

    return m_tiles.at(tileID)->interpolatedHeightAt(normX, normZ);
}

bool Terrain::worldToTileRowColumn(float x, float z, TerrainLevel level, std::shared_ptr<TerrainTile> & terrainTile, unsigned int & row, unsigned int & column) const
{
    // only implemented for 1 tile
    assert(settings.tilesX == 1 && settings.tilesZ == 1);
    float normX = (x / settings.sizeX + 0.5f);
    float normZ = (z / settings.sizeZ + 0.5f);
    bool valid = normX >= 0 && normX <= 1 && normZ >= 0 && normZ <= 1;

    row = static_cast<int>(normX * settings.rows) % settings.rows;
    column = static_cast<int>(normZ * settings.columns) % settings.columns;

    TileID tileID(level, 0, 0);

    terrainTile = m_tiles.at(tileID);

    assert(terrainTile);

    return valid;
}

bool Terrain::normalizePosition(float x, float z, TileID & tileID, float & normX, float & normZ) const
{
    // currently for one tile only
    assert(settings.tilesX == 1 && settings.tilesZ == 1);
    tileID.x = 0;
    tileID.z = 0;

    normX = x / settings.sizeX + 0.5f;
    normZ = z / settings.sizeZ + 0.5f;

    return normX >= 0.0f && normX <= 1.0f
        && normZ >= 0.0f && normZ <= 1.0f;
}
