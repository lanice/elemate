#include "hand.h"

#include <glow/Array.h>
#include <glow/logging.h>
#include <glow/VertexArrayObject.h>
#include <glow/VertexAttributeBinding.h>
#include <glow/Buffer.h>
#include <glow/Program.h>
#include <glowutils/File.h>
#include <glowutils/FileRegistry.h>
#include <glowutils/AxisAlignedBoundingBox.h>
#include "cameraex.h"

#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "world.h"
#include "terrain/terrain.h"
#include "rendering/shadowmappingstep.h"
#include "lua/luawrapper.h"

const std::string Hand::s_modelFilename = "data/models/hand.3DS";

Hand::Hand(const World & world)
: Drawable(world)
, m_numIndices(0)
, m_heightOffset(1.0f)
{
    setPosition(0.0f, 0.0f);

    loadModel();

    m_program = new glow::Program();
    m_program->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/hand.vert"),
        world.sharedShader(GL_FRAGMENT_SHADER, "shader/phongLighting.frag"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/hand.frag"));
}

void Hand::loadModel()
{
    // the model is much too large
    const glm::mat4 scale = glm::scale(glm::mat4(), glm::vec3(0.0005f));
    // the hand should "lay" on the ground
    const glm::mat4 rotate = glm::rotate(glm::rotate(glm::mat4(), 90.0f, glm::vec3(1.0f, .0f, .0f)), 180.0f, glm::vec3(.0f, 1.0f, .0f));
    // set the center ~ to the middle of the fingers
    const glm::mat4 translate = glm::translate(glm::mat4(), glm::vec3(0, 0, 1.2f));

    const glm::mat4 initTransform = translate * rotate * scale;

    Assimp::Importer importer;
    const aiScene * scene = importer.ReadFile(s_modelFilename, aiPostProcessSteps::aiProcess_Triangulate);
    assert(scene);
    if (!scene) {
        glow::fatal("Could not load hand model!");
        return;
    }

    assert(scene->mNumMeshes == 1);
    aiMesh * mesh = scene->mMeshes[0];

    glow::UIntArray * indices = new glow::UIntArray;
    for (unsigned face = 0; face < mesh->mNumFaces; ++face) {
        assert(mesh->mFaces[face].mNumIndices == 3);     // using triangles
        for (unsigned int i = 0; i < 3; ++i) {
            indices->push_back(mesh->mFaces[face].mIndices[i]);
        }
    }
    m_numIndices = static_cast<int>(indices->size());
    m_indexBuffer = new glow::Buffer(GL_ELEMENT_ARRAY_BUFFER);
    m_indexBuffer->setData(*indices, GL_STATIC_DRAW);
    m_indexBuffer->unbind();
    indices->clear();
    delete indices;

    glowutils::AxisAlignedBoundingBox bbox;
    glow::Vec3Array * vertices = new glow::Vec3Array;

    for (unsigned v = 0; v < mesh->mNumVertices; ++v) {
        const glm::vec3 scaledVertex = 
            glm::vec3(initTransform * glm::vec4(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z, 1.0));
        vertices->push_back(scaledVertex);
        bbox.extend(scaledVertex);
    }

    m_vbo = new glow::Buffer(GL_ARRAY_BUFFER);
    m_vbo->setData(*vertices, GL_STATIC_DRAW);
    m_vbo->unbind();
    vertices->clear();
    delete vertices;

    // use four lower coners of the bouding box as compare/checkpoints with terrain height
    m_heightCheckPoints.push_back(bbox.llf());
    m_heightCheckPoints.push_back(glm::vec3(bbox.llf().x, bbox.llf().y, bbox.urb().z));
    m_heightCheckPoints.push_back(glm::vec3(bbox.urb().x, bbox.llf().y, bbox.llf().z));
    m_heightCheckPoints.push_back(glm::vec3(bbox.urb().x, bbox.llf().y, bbox.urb().z));

    glow::Vec3Array * normals = new glow::Vec3Array;
    for (unsigned n = 0; n < mesh->mNumVertices; ++n) {
        glm::vec3 rotatedNormal = glm::vec3(rotate * glm::vec4(mesh->mNormals[n].x, mesh->mNormals[n].y, mesh->mNormals[n].z, 1.0));
        normals->push_back(rotatedNormal);
    }
    m_normalBuffer = new glow::Buffer(GL_ARRAY_BUFFER);
    m_normalBuffer->setData(*normals, GL_STATIC_DRAW);
    m_normalBuffer->unbind();
    normals->clear();
    delete normals;


    m_vao = new glow::VertexArrayObject;
    m_vao->bind();

    m_indexBuffer->bind();

    glow::VertexAttributeBinding * vertexBinding = m_vao->binding(0);
    vertexBinding->setAttribute(0);
    vertexBinding->setBuffer(m_vbo, 0, sizeof(glm::vec3));
    vertexBinding->setFormat(3, GL_FLOAT, GL_FALSE, 0);
    m_vao->enable(0);

    glow::VertexAttributeBinding * normalBinding = m_vao->binding(1);
    normalBinding->setAttribute(1);
    normalBinding->setBuffer(m_normalBuffer, 0, sizeof(glm::vec3));
    normalBinding->setFormat(3, GL_FLOAT, GL_FALSE, 0);
    m_vao->enable(1);

    m_vao->unbind();
}

float Hand::heightCheck(float worldX, float worldZ) const
{
    float maxHeight = std::numeric_limits<float>::lowest();
    float minHeight = std::numeric_limits<float>::max();

    assert(m_world.terrain);
    const Terrain & terrain(*m_world.terrain);

    for (const glm::vec3 & checkPoint : m_heightCheckPoints) {
        const glm::vec3 worldPos = glm::vec3(worldX, 0, worldZ) + xzTransform() * checkPoint;

        float heightAt = terrain.heightTotalAt(worldX + checkPoint.x, worldZ + checkPoint.z) + checkPoint.y;
        if (heightAt > maxHeight)
            maxHeight = heightAt;
        if (heightAt < minHeight)
            minHeight = heightAt;
    }

    float transition = (maxHeight + minHeight) * 0.5f + m_heightOffset;

    if (transition < maxHeight)
        transition = maxHeight;

    return transition;
}

void Hand::drawImplementation(const CameraEx & camera)
{
    m_program->use();
    m_program->setUniform("modelView", camera.view() * transform());
    m_program->setUniform("modelViewProjection", camera.viewProjectionEx() * transform());
    m_program->setUniform("rotate", m_rotate);
    m_program->setUniform("cameraposition", camera.eye());
    m_world.setUpLighting(*m_program);

    m_vao->drawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, nullptr);

    m_program->release();
}

const glm::mat4 & Hand::transform() const
{
    if (!m_transform.isValid())
        m_transform.setValue(translate() * m_rotate);

    return m_transform.value();
}

const glm::mat3 & Hand::xzTransform() const
{
    if (!m_xzTransform.isValid())
        m_xzTransform.setValue(glm::mat3(glm::translate(glm::mat4(), glm::vec3(m_positionX, 0.0f, m_positionZ)) * m_rotate));

    return m_xzTransform.value();
}

const glm::vec3 & Hand::position() const
{
    if (!m_positionY.isValid()) {
        m_positionY.setValue(heightCheck(m_positionX, m_positionZ));
        m_position.invalidate();
    }
    if (!m_position.isValid())
        m_position.setValue(glm::vec3(m_positionX, m_positionY.value(), m_positionZ));

    return m_position.value();
}

void Hand::setPosition(float worldX, float worldZ)
{
    if (m_positionX == worldX && m_positionZ == worldZ)
        return;

    m_positionX = worldX;
    m_positionZ = worldZ;

    m_positionY.invalidate();
    m_position.invalidate();

    m_xzTransform.invalidate();
    m_translate.invalidate();
    m_transform.invalidate();
}

const glm::mat4 & Hand::translate() const
{
    if (!m_translate.isValid())
        m_translate.setValue(glm::translate(glm::mat4(), position()));

    return m_translate.value();
}

void Hand::setHeightOffset(float heightOffset)
{
    if (m_heightOffset == heightOffset)
        return;

    m_heightOffset = heightOffset >= 0.0f ? heightOffset : 0.0f;

    m_positionY.invalidate();

    m_translate.invalidate();
    m_transform.invalidate();
}

float Hand::heightOffset() const
{
    return m_heightOffset;
}

void Hand::rotate(const float angle)
{
    m_rotate = glm::rotate(glm::mat4(), angle, glm::vec3(0.0f, 1.0f, 0.0f));
    m_transform.invalidate();
}

void Hand::drawDepthMapImpl(const CameraEx & camera)
{
    m_depthMapProgram->setUniform("depthMVP", camera.viewProjectionEx() * transform());
    m_depthMapProgram->setUniform("znear", camera.zNearEx());
    m_depthMapProgram->setUniform("zfar", camera.zFar());

    m_depthMapProgram->use();

    m_vao->drawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, nullptr);

    m_depthMapProgram->release();
}

void Hand::drawShadowMappingImpl(const CameraEx & camera, const CameraEx & lightSource)
{
    glm::mat4 lightBiasMVP = ShadowMappingStep::s_biasMatrix * lightSource.viewProjectionEx() * transform();

    m_shadowMappingProgram->setUniform("modelViewProjection", camera.viewProjectionEx() * transform());
    m_shadowMappingProgram->setUniform("lightBiasMVP", lightBiasMVP);

    m_vao->drawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, nullptr);
}

void Hand::initDepthMapProgram()
{
    m_depthMapProgram = new glow::Program();
    m_depthMapProgram->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/shadows/depthmap_hand.vert"),
        World::instance()->sharedShader(GL_FRAGMENT_SHADER, "shader/depth_util.frag"),
        World::instance()->sharedShader(GL_FRAGMENT_SHADER, "shader/passthrough.frag"));
}

void Hand::initShadowMappingProgram()
{
    m_shadowMappingProgram = new glow::Program();
    m_shadowMappingProgram->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/shadows/shadowmapping_hand.vert"),
        World::instance()->sharedShader(GL_FRAGMENT_SHADER, "shader/shadows/shadowmapping.frag"));

    ShadowMappingStep::setUniforms(*m_shadowMappingProgram);
}

void Hand::registerLuaFunctions(LuaWrapper * lua)
{
    std::function<float()> func0 = [=] ()
    { return position().x; };

    std::function<float()> func1 = [=]()
    { return position().y; };

    std::function<float()> func2 = [=]()
    { return position().z; };

    lua->Register("hand_posX", func0);
    lua->Register("hand_posY", func1);
    lua->Register("hand_posZ", func2);
}
