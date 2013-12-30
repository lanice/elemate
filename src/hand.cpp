#include "hand.h"

#include <glow/Array.h>
#include <glow/logging.h>
#include <glow/VertexArrayObject.h>
#include <glow/VertexAttributeBinding.h>
#include <glow/Buffer.h>
#include <glow/Program.h>
#include <glowutils/File.h>
#include <glowutils/Camera.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>


Hand::Hand()
{
    /*m_hand->addChild( osgDB::readNodeFile("data/models/hand.3DS") );
    m_transform->addChild( m_hand );

    _defaultTransform = (  osg::Matrixd::scale( 0.0005,0.0005,0.0005 ) * osg::Matrixd::rotate( 3.1415926f * 1.5, osg::Vec3(1.0, .0, .0) ) * osg::Matrixd::rotate( 3.1415926f * 1., osg::Vec3(.0, 1.0, .0) ) );

    m_transform->setMatrix( _defaultTransform );*/

    _defaultTransform = glm::mat4(0.0005f);
    _defaultTransform[3][3] = 1.0f;

    Assimp::Importer importer;
    const aiScene * scene = importer.ReadFile("data/models/hand.3DS", aiPostProcessSteps::aiProcess_Triangulate);
    assert(scene);
    if (!scene) {
        glow::fatal("Could not load hand model!");
        return;
    }

    aiMesh * mesh = scene->mMeshes[0];

    m_numVertices = mesh->mNumVertices;

    m_vao = new glow::VertexArrayObject;
    m_vbo = new glow::Buffer(GL_ARRAY_BUFFER);
    m_vbo->setData(m_numVertices, mesh->mVertices, GL_STATIC_DRAW);

    glow::VertexAttributeBinding * vertexBinding = m_vao->binding(0);
    vertexBinding->setAttribute(0);
    vertexBinding->setBuffer(m_vbo, 0, sizeof(glm::vec3));
    vertexBinding->setFormat(3, GL_FLOAT, GL_FALSE, 0);
    m_vao->enable(0);

    m_vao->unbind();

    m_program = new glow::Program();
    m_program->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/hand.vert"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/hand.frag"));

}


Hand::~Hand()
{
}


void Hand::draw(const glowutils::Camera & camera)
{
    m_program->use();
    m_program->setUniform("modelViewProjection", camera.viewProjection() * _defaultTransform);

    m_vao->bind();

    glPointSize(10.0f);
    m_vao->drawArrays(GL_POINTS, 0, m_numVertices);

    m_vao->unbind();

    m_program->release();
}


glm::mat4 Hand::transform()
{
    return m_transform;
}


glm::mat4 Hand::defaultTransform()
{
    return _defaultTransform;
}


glm::vec3 Hand::position()
{
    return glm::vec3();
    /*osg::Matrix matrix = m_transform.get()->getMatrix();
    return osg::Vec3( matrix(3, 0), matrix(3, 1), matrix(3, 2) );*/
}
