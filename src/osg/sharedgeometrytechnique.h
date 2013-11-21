#pragma once
#include <osgTerrain/GeometryTechnique>

namespace osgTerrain {

class SharedGeometryTechnique : public GeometryTechnique {
public:
    osg::ref_ptr<osg::Geometry> getGeometry() const;

    SharedGeometryTechnique();

    /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
    SharedGeometryTechnique(const SharedGeometryTechnique&, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

    META_Object(osgTerrain, SharedGeometryTechnique);
};


}
