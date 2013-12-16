#pragma once

#include <osgTerrain/TerrainTile>

#include <list>

class DynamicTerrainTile : public osgTerrain::TerrainTile
{
public:
    DynamicTerrainTile();

    /** set value in heightfield and apply it directly to the vertex data */
    void setHeightInGeometry(unsigned int column, unsigned int row, float newValue);

    /** pass the updated vertex buffer to the gpu */
    void updateVBO();

    /** retriangulate the geometry to recalculate the orientation of diagonales in quads between four height points */
    void retriangulate();

protected:
    osg::Vec3Array * m_currentVertices;
};
