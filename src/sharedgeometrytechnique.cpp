#include "sharedgeometrytechnique.h"

namespace osgTerrain {

SharedGeometryTechnique::SharedGeometryTechnique()
    : GeometryTechnique()
{}


SharedGeometryTechnique::SharedGeometryTechnique(const SharedGeometryTechnique& technique, const osg::CopyOp& copyop /*= osg::CopyOp::SHALLOW_COPY*/)
    : GeometryTechnique(technique, copyop)
{}

osg::ref_ptr<osg::Geometry> SharedGeometryTechnique::getGeometry() const
{
    if (_currentBufferData.valid())
        return _currentBufferData->_geometry;
    else
        return nullptr;
}

}
