#include "dynamicterraintile.h"

#include <osg/Shape>
#include <osgTerrain/Layer>
#include "osg/sharedgeometrytechnique.h"

#include <cassert>

DynamicTerrainTile::DynamicTerrainTile()
: m_currentVertices(nullptr)
{
}

void DynamicTerrainTile::setHeightInGeometry(unsigned int column, unsigned int row, float newValue)
{
    // this is ugly, but fast

    // Make some assumptions on tile configuration...
    assert(_elevationLayer.valid());
    assert(dynamic_cast<osgTerrain::HeightFieldLayer*>(_elevationLayer.get()));
    osg::ref_ptr<osg::HeightField> hf = static_cast<osgTerrain::HeightFieldLayer*>(_elevationLayer.get())->getHeightField();
    assert(hf.valid());

    hf->setHeight(column, row, newValue);

    if (m_currentVertices == nullptr) {
        assert(dynamic_cast<SharedGeometryTechnique*>(_terrainTechnique.get()));
        SharedGeometryTechnique * geometryTechnique = static_cast<SharedGeometryTechnique*>(_terrainTechnique.get());
        assert(geometryTechnique->getGeometry());
        assert(geometryTechnique->getGeometry()->getVertexArray());
        assert(dynamic_cast<osg::Vec3Array*>(geometryTechnique->getGeometry()->getVertexArray()));

        m_currentVertices = static_cast<osg::Vec3Array*>(geometryTechnique->getGeometry()->getVertexArray());
    }

    // make some assumptions on osg's implementations...
    // that's the value we want to change, right?
    (*m_currentVertices)[column + row * hf->getNumColumns()][2] = newValue;
}

void DynamicTerrainTile::updateVBO()
{
    m_currentVertices->dirty();

    m_currentVertices = nullptr;
}

void DynamicTerrainTile::retriangulate()
{
    setDirty(true);
    osgUtil::UpdateVisitor uv;
    uv.apply(*this);
}
