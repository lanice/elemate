#include "terraininteractor.h"

#include <iostream>

#include <osgTerrain/Terrain>
#include <osgTerrain/Layer>
#include <osgUtil/UpdateVisitor>

#include "terrain/elemateterrain.h"


TerrainInteractor::TerrainInteractor(std::shared_ptr<ElemateHeightFieldTerrain>& terrain)
: m_terrain(terrain)
{
}

float TerrainInteractor::setHeight(float worldX, float worldZ, TerrainLevel level, float value)
{
    osg::ref_ptr<osgTerrain::TerrainTile> tile;
    unsigned int physxRow;
    unsigned int physxColumn;

    if (!m_terrain->worldToTileRowColumn(worldX, worldZ, level, tile, physxRow, physxColumn))
        return 0.0f;

    return setHeight(*tile.get(), physxRow, physxColumn, value);
}

float TerrainInteractor::changeHeight(float worldX, float worldZ, TerrainLevel level, float delta)
{
    osg::ref_ptr<osgTerrain::TerrainTile> tile;
    unsigned int physxRow;
    unsigned int physxColumn;

    if (!m_terrain->worldToTileRowColumn(worldX, worldZ, level, tile, physxRow, physxColumn))
        return 0.0f;

    float height = m_terrain->heightAt(*tile.get(), physxRow, physxColumn);

    return setHeight(*tile.get(), physxRow, physxColumn, height + delta);
}

float TerrainInteractor::setHeight(osgTerrain::TerrainTile & tile, unsigned physxRow, unsigned physxColumn, float value)
{
    const TerrainSettings & settings = m_terrain->settings();
    unsigned int osgColumn = physxRow;
    unsigned int osgRow = settings.columns - physxColumn - 1;

    osg::ref_ptr<osgTerrain::HeightFieldLayer> hfLayer = dynamic_cast<osgTerrain::HeightFieldLayer*>(tile.getElevationLayer());
    assert(hfLayer.valid());
    
    float value_inRange = value; /** clamp height value */
    if (value_inRange < -settings.maxHeight) value_inRange = -settings.maxHeight;
    if (value_inRange > settings.maxHeight) value_inRange = settings.maxHeight;

    hfLayer->getHeightField()->setHeight(osgColumn, osgRow, value_inRange);

    tile.setDirty(true);
    osgUtil::UpdateVisitor uv;
    uv.apply(tile);

    return value_inRange;
}

std::shared_ptr<ElemateHeightFieldTerrain> TerrainInteractor::terrain() const
{
    return m_terrain;
}

void TerrainInteractor::setTerrain(std::shared_ptr<ElemateHeightFieldTerrain>& terrain)
{
    m_terrain = terrain;
}
