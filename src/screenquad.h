#pragma once

#include <vector>

#include <osg/Drawable>

namespace osg {
    class Texture2D;
}

class ScreenQuad : public osg::Drawable
{
public:
    /** creates a screen quad drawable, adds it to a geode and applies the flush shader.
      * @return pointer to the geode */
    static osg::Geode * createFlushNode(osg::Texture2D & colorBuffer, osg::Program & flushProgram);

    ScreenQuad();
    virtual ~ScreenQuad() override;

    virtual void drawImplementation(osg::RenderInfo& renderInfo) const override;

    virtual osg::BoundingBox computeBound() const override;

protected:
    mutable bool m_needGLUpdate;
    mutable GLuint m_vbo;
    void updateGLObjects(osg::RenderInfo& renderInfo) const;

    virtual void releaseGLObjects(osg::State* state) const override;

    virtual inline osg::Object * clone(const osg::CopyOp& /*copyop*/) const override {
        return nullptr;
    }
    virtual inline osg::Object * cloneType() const override {
        return nullptr;
    }

};
