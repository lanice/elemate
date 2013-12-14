#include "screenquad.h"

#include <osg/Geode>
#include <osg/Texture2D>

osg::Geode * ScreenQuad::createFlushNode(osg::Texture2D & colorBuffer, osg::Program & flushProgram)
{
    ScreenQuad * quad = new ScreenQuad();
    osg::Geode * geode = new osg::Geode();
    geode->addDrawable(quad);
    geode->getOrCreateStateSet()->setAttributeAndModes(&flushProgram, osg::StateAttribute::ON);
    geode->getOrCreateStateSet()->setTextureAttributeAndModes(0, &colorBuffer, osg::StateAttribute::ON);

    return geode;
}

ScreenQuad::ScreenQuad()
: m_vbo(UINT_MAX)
{
}

ScreenQuad::~ScreenQuad()
{
}

void ScreenQuad::drawImplementation(osg::RenderInfo& renderInfo) const
{
    if (m_needGLUpdate)
        updateGLObjects(renderInfo);

    osg::State & state = *renderInfo.getState();
    Extensions & ext = *getExtensions(renderInfo.getContextID(), true);

    ext.glBindBuffer(GL_ARRAY_BUFFER_ARB, m_vbo);
    state.setVertexPointer(2, GL_FLOAT, 0, 0);

    state.glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 0);

    state.disableAllVertexArrays();

    return;
}

void ScreenQuad::updateGLObjects(osg::RenderInfo& renderInfo) const
{
    static const GLfloat vertices[] =
    {
          +1.f, -1.f
        , +1.f, +1.f
        , -1.f, -1.f
        , -1.f, +1.f
    };

    Extensions & ext = *getExtensions(renderInfo.getContextID(), true);

    if (m_vbo == UINT_MAX) {
        ext.glGenBuffers(1, &m_vbo);
    }

    // copy vertex data to the gpu
    ext.glBindBuffer(GL_ARRAY_BUFFER_ARB, m_vbo);
    ext.glBufferData(GL_ARRAY_BUFFER_ARB, 8, vertices, GL_STATIC_DRAW);
    ext.glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);

    m_needGLUpdate = false;
}

void ScreenQuad::releaseGLObjects(osg::State* state) const
{
    if (m_vbo != UINT_MAX) {
        if (state)
            getExtensions(state->getContextID(), true)->glDeleteBuffers(1, &m_vbo);
    }
    Drawable::releaseGLObjects(state);
}

osg::BoundingBox ScreenQuad::computeBound() const
{
    return osg::BoundingBox();
}
