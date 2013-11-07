#pragma once

#include <vector>

#include <osg/Drawable>
#include <osg/Geode>

class WorldDrawable : public osg::Drawable
{
public:
    WorldDrawable();

    virtual inline osg::Object * clone(const osg::CopyOp& copyop) const override{
        return new WorldDrawable();
    }
    virtual inline osg::Object * cloneType() const override {
        return new WorldDrawable();
    }

    // this one is important :-D
    virtual void drawImplementation(osg::RenderInfo& renderInfo) const override;

    virtual osg::BoundingBox computeBound() const override;

protected:
    void doLazyInitialize(osg::RenderInfo & renderInfo) const;
    void initialize(osg::RenderInfo & renderInfo);
    osg::Program * reloadShaders();

    bool m_isComplete;
    GLuint m_vertexBuffer;
    std::vector<osg::Vec3f> m_vertices;
};