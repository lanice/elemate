#include "hand.h"

#include <glow/Array.h>
#include <glow/logging.h>
#include <glow/VertexArrayObject.h>
#include <glow/VertexAttributeBinding.h>
#include <glow/Buffer.h>
#include <glow/Program.h>
#include <glowutils/File.h>
#include <glowutils/Camera.h>

#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "world.h"


Hand::Hand(const World & world)
: m_world(world)
{
    m_rotate = glm::rotate(glm::mat4(), 90.0f, glm::vec3(1.0f, .0f, .0f));
    m_rotate = glm::rotate(m_rotate, 180.0f, glm::vec3(.0f, 1.0f, .0f));

    m_scale = glm::scale(glm::mat4(), glm::vec3(0.0005f));

    setPosition(glm::vec3());

    Assimp::Importer importer;
    const aiScene * scene = importer.ReadFile("data/models/hand.3DS", aiPostProcessSteps::aiProcess_Triangulate);
    assert(scene);
    if (!scene) {
        glow::fatal("Could not load hand model!");
        return;
    }

    assert(scene->mNumMeshes == 1);
    aiMesh * mesh = scene->mMeshes[0];

    m_numVertices = mesh->mNumVertices;
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
    delete indices;

    glow::Vec3Array * vertices = new glow::Vec3Array;
    for (unsigned v = 0; v < mesh->mNumVertices; ++v) {
        vertices->push_back(glm::vec3(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z));
    }
    m_vbo = new glow::Buffer(GL_ARRAY_BUFFER);
    m_vbo->setData(*vertices, GL_STATIC_DRAW);
    m_vbo->unbind();
    delete vertices;

    glow::Vec3Array * normals = new glow::Vec3Array;
    for (unsigned n = 0; n < mesh->mNumVertices; ++n) {
        normals->push_back(glm::vec3(mesh->mNormals[n].x, mesh->mNormals[n].y, mesh->mNormals[n].z));
    }
    m_normalBuffer = new glow::Buffer(GL_ARRAY_BUFFER);
    m_normalBuffer->setData(*normals, GL_STATIC_DRAW);
    m_normalBuffer->unbind();
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


    m_program = new glow::Program();
    m_program->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/hand.vert"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/phongLighting.frag"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/hand.frag"));
}


Hand::~Hand()
{
}


void Hand::draw(const glowutils::Camera & camera)
{
    m_program->use();
    m_program->setUniform("modelView", camera.view() * m_transform);
    m_program->setUniform("modelViewProjection", camera.viewProjection() * m_transform);
    m_program->setUniform("rotate", m_rotate);
    m_program->setUniform("cameraposition", camera.eye());
    m_world.setUpLighting(*m_program);
    
    m_vao->bind();

    glPointSize(10.0f);
    m_vao->drawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, nullptr);

    m_vao->unbind();

    m_program->release();
}


glm::mat4 Hand::transform() const
{
    return m_transform;
}

glm::vec3 Hand::position() const
{
    return m_position;
}

void Hand::setPosition(const glm::vec3 & position)
{
    m_position = position;
    m_translate = glm::mat4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        position.x, position.y, position.z, 1.0f);
    m_transform = m_translate * m_rotate * m_scale;
}
