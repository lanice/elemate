#include "terraininteractor.h"

#include <iostream>

#include <osgTerrain/Terrain>
#include <osgTerrain/Layer>

#include "terrain/elemateterrain.h"


TerrainInteractor::TerrainInteractor(std::shared_ptr<ElemateHeightFieldTerrain>& terrain)
: m_terrain(terrain)
{
}

float TerrainInteractor::setHeight(float worldX, float worldZ, TerrainLevel level, float value)
{
    // assuming to have only one tile
    assert(m_terrain->osgTerrainBase()->getNumChildren() == 1);
    const TerrainSettings & settings = m_terrain->settings(); // just for shorter code

    float value_inRange = value; /** clamp height value */
    if (value_inRange <= -settings.maxHeight) value_inRange = -settings.maxHeight;
    if (value_inRange > settings.maxHeight) value_inRange = settings.maxHeight;

    /** determine row/column position */
    unsigned int physxRow;
    unsigned int physxColumn;
    osgTerrain::TileID tileID;
    assert(false);
    /*if (!m_terrain->worldToTileRowColumn(worldX, worldZ, tileID, physxRow, physxColumn))
        return 0.0f;

    tileID.level = static_cast<int>(level);

    unsigned int osgColumn = physxRow;
    unsigned int osgRow = settings.columns - physxColumn - 1;

    osg::ref_ptr<osgTerrain::HeightFieldLayer> hfLayer = dynamic_cast<osgTerrain::HeightFieldLayer*>
        (m_terrain->osgTerrain()->getTile(tileID)->getElevationLayer());
    assert(hfLayer.valid());

    std::cout << "pos: " << worldX << "x" << worldZ <<
        "row/Column: " << physxRow << "/" << physxColumn << std::endl <<
        "old/new/inRange: " << hfLayer->getHeightField()->getHeight(osgColumn, osgRow) <<
            "/"<< value << "/" <<value_inRange << std::endl;

    hfLayer->getHeightField()->setHeight(osgColumn, osgRow, value_inRange);

    hfLayer->dirty();*/

    return value_inRange;
}

float TerrainInteractor::changeHeight(float worldX, float worldZ, TerrainLevel level, float delta)
{
    return setHeight(worldX, worldZ, level,
        m_terrain->heightAt(worldX, worldZ, level) + delta);
}

std::shared_ptr<ElemateHeightFieldTerrain> TerrainInteractor::terrain() const
{
    return m_terrain;
}

void TerrainInteractor::setTerrain(std::shared_ptr<ElemateHeightFieldTerrain>& terrain)
{
    m_terrain = terrain;
}
