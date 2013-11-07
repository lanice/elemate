#include "worlddrawable.h"

#include <osg/Program>
#include <osgDB/ReadFile>

#include <osgUtil/DelaunayTriangulator>

// coordinates according to navigation:
// x is right
// y is up
// -z is front

WorldDrawable::WorldDrawable()
: m_isComplete(false)
, m_vertexBuffer(-1)
, m_vertices(
{osg::Vec3f(-1.0f, 0.0f, -1.0f),
osg::Vec3f(-1.0f, 0.0f, 1.0f),
osg::Vec3f(1.0f, 0.0f, 1.0f),
osg::Vec3f(1.0f, 0.0f, -1.0f),
osg::Vec3f(-1.0f, 0.0f, -1.0f)})
{
    setUseDisplayList(false);
}

void WorldDrawable::drawImplementation(osg::RenderInfo & renderInfo) const
{
    // hack around constness, to use lazy initialize
    doLazyInitialize(renderInfo);

    osg::State & state = *renderInfo.getState();
    Extensions & ext = *getExtensions(renderInfo.getContextID(), true);

    ext.glBindBuffer(GL_ARRAY_BUFFER_ARB, m_vertexBuffer);
    state.setVertexPointer(3, GL_FLOAT, 0, 0);

    state.glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, m_vertices.size(), 0);

    state.disableAllVertexArrays();

}

void WorldDrawable::doLazyInitialize(osg::RenderInfo & renderInfo) const
{
    return (const_cast<WorldDrawable*>(this))->initialize(renderInfo);
}

void WorldDrawable::initialize(osg::RenderInfo & renderInfo)
{
    osg::StateSet * stateSet = getOrCreateStateSet();
    //stateSet->addUniform(new osg::Uniform("namederuniform", osg::Vec3(0.3, 0.1, 1.0)));

    if (m_isComplete)
        return;

    Extensions & ext = *getExtensions(renderInfo.getContextID(), true);

    ext.glGenBuffers(1, &m_vertexBuffer);
    ext.glBindBuffer(GL_ARRAY_BUFFER_ARB, m_vertexBuffer);
    ext.glBufferData(GL_ARRAY_BUFFER_ARB, m_vertices.size()*sizeof(float)*3, m_vertices.data(), GL_STATIC_DRAW);

    stateSet->setAttributeAndModes( reloadShaders() );

    m_isComplete = true;
}

osg::Program * WorldDrawable::reloadShaders()
{
    osg::ref_ptr<osg::Shader> vertexShader =
        osgDB::readShaderFile("data/world.vert");
    osg::ref_ptr<osg::Shader> fragmentShader =
        osgDB::readShaderFile("data/world.frag");

    osg::Program * program = new osg::Program();
    program->addShader(vertexShader.get());
    program->addShader(fragmentShader.get());
    return program;
}

osg::BoundingBox WorldDrawable::computeBound() const
{
    osg::BoundingBox bb;
    for (const auto & v : m_vertices)
        bb.expandBy(v);
    return bb;
}
