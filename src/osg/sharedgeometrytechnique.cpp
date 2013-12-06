#include "sharedgeometrytechnique.h"

using namespace osgTerrain;

SharedGeometryTechnique::SharedGeometryTechnique()
    : GeometryTechnique()
{}


SharedGeometryTechnique::SharedGeometryTechnique(const SharedGeometryTechnique& technique, const osg::CopyOp& copyop /*= osg::CopyOp::SHALLOW_COPY*/)
    : GeometryTechnique(technique, copyop)
{}

osg::Geometry * SharedGeometryTechnique::getGeometry() const
{
    if (_currentBufferData.valid())
        return _currentBufferData->_geometry.get();
    else
        return nullptr;
}
