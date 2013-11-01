#include "worlddrawable.h"

#include <osg/Program>
#include <osgDB/ReadFile>

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

    osg::State * state = renderInfo.getState();
    Extensions * ext = getExtensions(renderInfo.getContextID(), true);

    ext->glBindBuffer(GL_ARRAY_BUFFER_ARB, m_vertexBuffer);
    state->setVertexPointer(3, GL_FLOAT, 0, 0);

    state->glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, m_vertices.size(), 0);

    state->disableAllVertexArrays();
    
}

void WorldDrawable::doLazyInitialize(osg::RenderInfo & renderInfo) const
{
    return (const_cast<WorldDrawable*>(this))->initialize(renderInfo);
}

void WorldDrawable::initialize(osg::RenderInfo & renderInfo)
{
    if (m_isComplete)
        return;

    Extensions * ext = getExtensions(renderInfo.getContextID(), true);

    ext->glGenBuffers(1, &m_vertexBuffer);
    ext->glBindBuffer(GL_ARRAY_BUFFER_ARB, m_vertexBuffer);
    ext->glBufferData(GL_ARRAY_BUFFER_ARB, m_vertices.size()*sizeof(float)*3, m_vertices.data(), GL_STATIC_DRAW);

    //reloadShaders(*renderInfo.getState());

    //osg::StateSet * stateSet = getOrCreateStateSet();
    //stateSet->setAttributeAndModes(m_program.get());

    m_isComplete = true;
}

//void WorldDrawable::reloadShaders(osg::State & state)
//{
//    osg::ref_ptr<osg::Shader> vertexShader = new osg::Shader(osg::Shader::VERTEX);
//    vertexShader->loadShaderSourceFromFile("C:/develop/elemate/data/world.vert");
//    osg::ref_ptr<osg::Shader> fragmentShader = new osg::Shader(osg::Shader::FRAGMENT);
//    fragmentShader->loadShaderSourceFromFile("C:/develop/elemate/data/world.frag");
//
//    // i think there are some osg functions to reload the shader...
//    if (m_program.get() != nullptr)
//        m_program.release();
//
//    m_program = new osg::Program();
//    m_program->addShader(vertexShader.get());
//    m_program->addShader(fragmentShader.get());
//    m_program->apply(state);
//}

osg::BoundingBox WorldDrawable::computeBound() const
{
    osg::BoundingBox bb;
    for (const auto & v : m_vertices)
        bb.expandBy(v);
    return bb;
    //return osg::BoundingBox(osg::Vec3(-1, -1, -1), osg::Vec3(1, 1, 1));
}
