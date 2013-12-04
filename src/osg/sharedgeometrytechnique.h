#pragma once
#include <osgTerrain/GeometryTechnique>

class SharedGeometryTechnique : public osgTerrain::GeometryTechnique {
public:
    osg::Geometry * getGeometry() const;

    SharedGeometryTechnique();

    SharedGeometryTechnique(const SharedGeometryTechnique&, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

    META_Object(osgTerrain, SharedGeometryTechnique);
};

