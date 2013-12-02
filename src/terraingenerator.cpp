#include "terraingenerator.h"

#include <random>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <limits>
#include <cmath>
#include <ctime>

#include <osg/MatrixTransform>
#include <osg/Image>
#include <osgTerrain/Terrain>
#include <osgUtil/GLObjectsVisitor>
#include "osg/sharedgeometrytechnique.h"

#include <PxRigidStatic.h>
#include <PxShape.h>
#include <geometry/PxHeightField.h>
#include <geometry/PxHeightFieldSample.h>
#include <geometry/PxHeightFieldDesc.h>
#include <geometry/PxHeightFieldGeometry.h>
#include <foundation/PxMat44.h>

// Mersenne Twister, preconfigured
// keep one global instance, !per thread!

std::mt19937 rng;

using namespace physx;

namespace {
    uint32_t seed_val = std::time(0);
    bool initRng() {
        rng.seed(seed_val);
        return true;
    }
    bool didRngInit = initRng();
}

TerrainSettings::TerrainSettings()
: sizeX(200)
, sizeZ(200)
, rows(20)
, columns(20)
, tilesX(1)
, tilesZ(1)
, maxHeight(2.f)
, biomeSize(5.0f)
{
}

TerrainGenerator::TerrainGenerator()
: m_artifact(nullptr)
{
}

ElemateHeightFieldTerrain * TerrainGenerator::generate() const
{
    assert(m_artifact == nullptr);

    m_artifact = new ElemateHeightFieldTerrain(m_settings);

    osg::ref_ptr<osgTerrain::Terrain> osgTerrain = new osgTerrain::Terrain();
    osgTerrain->setName("terrain root node");
    // SharedGeometryTechnique allows us to access the terrain geometry (and add additional vertex attributes)
    osgTerrain->setTerrainTechniquePrototype(new SharedGeometryTechnique());
    m_artifact->m_osgTerrain = osgTerrain.get();

    /** transforms osg's base vectors to physx/opengl logic
      * ! matrix is inverted, in osg logic
      * osg base in px: x is x, y is -z, z is y. */
    osg::Matrix terrainBaseOsgToPhysx(
        1, 0, 0, 0,
        0, 0, -1, 0,
        0, 1, 0, 0,
        0, 0, 0, 1);

    m_artifact->m_osgTerrainTransform = new osg::MatrixTransform(terrainBaseOsgToPhysx);
    m_artifact->m_osgTerrainTransform->addChild(osgTerrain.get());


    /** The tileID determines the position of the current tile in the grid of tiles.
      * Tiles get shifted by -(numTilesPerAxis + 1)/2 so that we have the Tile(0,0,0) in the origin.
      */
    
    int maxxID = m_settings.tilesX - int((m_settings.tilesX + 1) * 0.5);
    int minxID = maxxID - m_settings.tilesX + 1;
    int maxzID = m_settings.tilesZ - int((m_settings.tilesZ + 1) * 0.5);
    int minzID = maxzID - m_settings.tilesZ + 1;

    for (int xID = minxID; xID <= maxxID; ++xID)
    for (int zID = minzID; zID <= maxzID; ++zID)
    {
        osgTerrain::TileID tileID(0, xID, zID);

        /** 1. Create terrain heightfield data.
            2. Add terrain type id / biome type information to osg geometry and physx heightfield samples.
            3. Create osg geometry (in osg TerrainTile) from this heightfield data. 
            4. Create physx actor with complete terrain information (heightfield + terrain type). */

        PxHeightFieldSample * pxHeightFieldSamples = createPxHeightFieldData(m_settings.rows * m_settings.columns);
        assert(pxHeightFieldSamples);

        /** create OSG terrain object and copy PhysX terrain data into it */

        osg::ref_ptr<osgTerrain::TerrainTile> tile = createTile(tileID, pxHeightFieldSamples);
        assert(tile.valid());
        tile->setTerrain(m_artifact->osgTerrain()); // tell the tile that it's part of our terrain object
        osgTerrain->addChild(tile.get());


        /** Adds vertex attribute array to terrain geometry, containing terrain type ids.
        Writes terrain type id to phyxs heightfield samples. */
        createBiomes(*tile.get(), pxHeightFieldSamples);

        /** tile gets a clone of terrains SharedGeometryTechnique and creates its geometry in OpenGL objects */
        osg::ref_ptr<osgUtil::GLObjectsVisitor> geoVisitor =
            new osgUtil::GLObjectsVisitor(osgUtil::GLObjectsVisitor::ModeValues::SWITCH_ON_VERTEX_BUFFER_OBJECTS);
        geoVisitor->apply(*tile.get());


        /** move tile according to its id, and by one half tile size, so the center of Tile(0,0,0) is in the origin */
        PxTransform pxTerrainTransform = PxTransform(PxVec3(m_settings.tileSizeX() * (xID - 0.5), 0.0f, m_settings.tileSizeZ() * (zID - 0.5)));
        PxRigidStatic * actor = PxGetPhysics().createRigidStatic(pxTerrainTransform);
        m_artifact->m_pxActors.emplace(tileID, actor);

        PxShape * pxShape = createPxShape(*actor, pxHeightFieldSamples, pxTerrainTransform);
        m_artifact->m_pxShapes.emplace(tileID, pxShape);


        //// debug the height geometries

        //osgTerrain::HeightFieldLayer * osgHeightField = dynamic_cast<osgTerrain::HeightFieldLayer*>(tile->getElevationLayer());
        //assert(osgHeightField);
        //std::cout << "OSG x(right)columns y(front)row z(up)" << std::endl;
        //std::cout << std::setprecision(1);
        //std::cout << std::fixed;
        //osg::Matrix transformOsgComlete = osgHeightField->getLocator()->getTransform() * osgBaseTransformToPx;
        //// use osg way of columns and rows here
        //for (unsigned c = 0; c < m_settings.rows; ++c) {
        //    std::cout << "Column " << c << std::endl;
        //    for (unsigned r = 0; r < m_settings.columns; ++r) {
        //        //osg::Vec3 o = osgHeightField->getHeightField()->getOrigin();
        //        osg::Vec3 v = osgHeightField->getHeightField()->getVertex(c, r);
        //        osg::Vec3 vt = osg::Vec3(c, r, 0) * transformOsgComlete;
        //        float height;
        //        osgHeightField->getInterpolatedValue(c / m_settings.rows, r / m_settings.columns, height);
        //        vt[2] = height;
        //        std::cout.width(5); std::cout << v.x() << " ";
        //        std::cout.width(5); std::cout << v.y() << " ";
        //        std::cout.width(5); std::cout << v.z() << "     ";

        //        std::cout.width(5); std::cout << vt.x() << " ";
        //        std::cout.width(5); std::cout << vt.y() << " ";
        //        std::cout.width(5); std::cout << vt.z() << std::endl;
        //    }
        //    std::cout << std::endl;
        //}
    }


    /** set some uniforms for the terrain */
    osg::ref_ptr<osg::StateSet> terrainStateSet = m_artifact->m_osgTerrain->getOrCreateStateSet();
    // texture unit 0 should be color layer 0 in all tiles
    osg::ref_ptr<osg::Uniform> terrainIDSampler = new osg::Uniform(osg::Uniform::Type::SAMPLER_2D, "terrainID");
    terrainIDSampler->set(0);
    terrainStateSet->addUniform(terrainIDSampler.get());
    terrainStateSet->addUniform(new osg::Uniform("tileSize", osg::Vec3(
        m_settings.tileSizeX(),
        m_settings.maxHeight,
        m_settings.tileSizeZ())));
    terrainStateSet->addUniform(new osg::Uniform("tileRowsColumns", osg::Vec2(m_settings.rows, m_settings.columns))); // ivec2


    ElemateHeightFieldTerrain * completeTerrain = m_artifact;
    m_artifact = nullptr;

    return completeTerrain;
}

void TerrainGenerator::setExtentsInWorld(float x, float z)
{
    assert(x > 0 && z > 0);
    m_settings.sizeX = x;
    m_settings.sizeZ = z;
}

float TerrainGenerator::xExtens() const
{
    return m_settings.sizeX;
}

float TerrainGenerator::zExtens() const
{
    return m_settings.sizeZ;
}

void TerrainGenerator::applySamplesPerWorldCoord(float xzSamplesPerCoord)
{
    assert(xzSamplesPerCoord > 0.0f);
    unsigned int xSamplesui = unsigned int(ceil(m_settings.sizeX * xzSamplesPerCoord));
    m_settings.rows = xSamplesui >= 2 ? xSamplesui : 2;
    unsigned int zSamplesui = unsigned int(ceil(m_settings.sizeZ * xzSamplesPerCoord));
    m_settings.columns = zSamplesui >= 2 ? zSamplesui : 2;
}

float TerrainGenerator::samplesPerWorldXCoord() const
{
    return m_settings.samplesPerXCoord();
}

float TerrainGenerator::samplesPerWorldZCoord() const
{
    return m_settings.samplesPerZCoord();
}

void TerrainGenerator::setTilesPerAxis(unsigned x, unsigned z)
{
    assert(x >= 1 && z >= 1);
    m_settings.tilesX = x;
    m_settings.tilesZ = z;
}

int TerrainGenerator::tilesPerXAxis() const
{
    return m_settings.tilesX;
}

int TerrainGenerator::tilesPerZAxis() const
{
    return m_settings.tilesZ;
}

void TerrainGenerator::setMaxHeight(float height)
{
    assert(height > 0.0f);
    m_settings.maxHeight = height;
}
float TerrainGenerator::maxHeight() const
{
    return m_settings.maxHeight;
}

void TerrainGenerator::setBiomeSize(float xzBiomeSize)
{
    assert(xzBiomeSize > 0);
    m_settings.biomeSize = xzBiomeSize;
}

float TerrainGenerator::biomeSize() const
{
    return m_settings.biomeSize;
}

PxHeightFieldSample * TerrainGenerator::createPxHeightFieldData(unsigned numSamples) const
{
    assert(m_settings.rows >= 2);
    assert(m_settings.columns >= 2);
    assert(m_settings.maxHeight > 0);

    std::uniform_int_distribution<> uniform_dist(
        std::numeric_limits<PxI16>::min(),
        std::numeric_limits<PxI16>::max());

    // physx stores values in row major order (means starting with all values (per column) for the first row)
    PxHeightFieldSample* hfSamples = new PxHeightFieldSample[numSamples];
    for (unsigned i = 0; i < numSamples; ++i) {
        hfSamples[i].materialIndex0 = 0;    // this clears also the tessellation flag
        hfSamples[i].materialIndex1 = 0;
        hfSamples[i].height = uniform_dist(rng);
    }
    return hfSamples;
}

PxShape * TerrainGenerator::createPxShape(PxRigidStatic & pxActor, const PxHeightFieldSample * hfSamples, const PxMat44 & transform) const
{
    PxHeightFieldDesc hfDesc;
    hfDesc.format = PxHeightFieldFormat::eS16_TM;
    hfDesc.nbRows = m_settings.rows;
    hfDesc.nbColumns = m_settings.columns;
    hfDesc.samples.data = hfSamples;
    hfDesc.samples.stride = sizeof( PxHeightFieldSample );

    PxHeightField * pxHeightField = PxGetPhysics().createHeightField(hfDesc);

    PxMaterial * mat[1];
    mat[0] = PxGetPhysics().createMaterial(0.5f, 0.5f, 0.1f);

    // scale height so that we use the full range of PxI16=short
    PxReal heightScale = m_settings.maxHeight / (-std::numeric_limits<PxI16>::min());
    assert(m_settings.intervalX() >= PX_MIN_HEIGHTFIELD_XZ_SCALE);
    assert(m_settings.intervalZ() >= PX_MIN_HEIGHTFIELD_XZ_SCALE);
    assert(heightScale >= PX_MIN_HEIGHTFIELD_Y_SCALE);
    // create height field geometry and set scale
    PxHeightFieldGeometry * m_pxHfGeometry = new  PxHeightFieldGeometry(pxHeightField, PxMeshGeometryFlags(),
        heightScale, m_settings.intervalX(), m_settings.intervalZ());
    PxShape * shape = pxActor.createShape(*m_pxHfGeometry, mat, 1);

    //// debug the height geometries
    //std::cout << "phyx: x(right)rows y(up) z(back)columns" << std::endl;
    //std::cout << std::setprecision(1);
    //std::cout << std::fixed;

    //for (unsigned r = 0; r < hfDesc.nbRows; ++r) {
    //    std::cout << "Row: " << r << std::endl;
    //    for (unsigned c = 0; c < hfDesc.nbColumns; ++c) {
    //        PxVec3 v = PxVec3(
    //            PxReal(r) * m_pxHfGeometry->rowScale,
    //            PxReal(hfSamples[c + ( r*hfDesc.nbColumns )].height) * m_pxHfGeometry->heightScale,
    //            PxReal(c) * m_pxHfGeometry->columnScale);
    //        PxVec3 vt = transform.transform(v);
    //        std::cout.width(5); std::cout << v.x << " ";
    //        std::cout.width(5); std::cout << v.y << " ";
    //        std::cout.width(5); std::cout << v.z << "     ";

    //        std::cout.width(5); std::cout << vt.x << " ";
    //        std::cout.width(5); std::cout << vt.y << " ";
    //        std::cout.width(5); std::cout << vt.z << std::endl;
    //    }
    //    std::cout << std::endl;
    //}

    assert(shape);

    return shape;
}

osgTerrain::TerrainTile * TerrainGenerator::createTile(const osgTerrain::TileID & tileID, const PxHeightFieldSample * pxHfSamples) const
{
    osg::ref_ptr<osg::HeightField> heightField = new osg::HeightField;
    // invert columns <-> rows to match with physx
    heightField->allocate(m_settings.rows, m_settings.columns);

    float heightScale = m_settings.maxHeight / (-std::numeric_limits<PxI16>::min());

    // osg column == physx row
    // osg row == numColumns - physx column - 1 (osg rows going to y, physx rows to z, where osgY == -physxZ)
    for (unsigned physxRow = 0; physxRow < m_settings.rows; ++physxRow) {
        unsigned rowOffset = physxRow * m_settings.columns;
        for (unsigned physxColumn = 0; physxColumn < m_settings.columns; ++physxColumn) {
            // set osg heightfield data, matching the physx rows/columns
            // physx samples in row major order
            heightField->setHeight(physxRow, m_settings.columns - physxColumn - 1,
                PxReal(pxHfSamples[physxColumn + rowOffset].height) * heightScale);
        }
    }

    osg::ref_ptr<osgTerrain::Locator> locator = new osgTerrain::Locator();

    // compute position depending on TileID, which sets the row/column positions of the tile
    // using x/z here, which corresponds to the world coordinates used in physx etc
    float minX = m_settings.tileSizeX() * (tileID.x - 0.5);
    float minZ = m_settings.tileSizeZ() * (tileID.y - 0.5);
    locator->setTransform(osg::Matrixd(
        m_settings.tileSizeX(), 0, 0, 0,
        0, m_settings.tileSizeZ(), 0, 0,
        0, 0, 1, 0,
        minX, minZ, 0, 1));

    osg::ref_ptr<osgTerrain::HeightFieldLayer> layer = new osgTerrain::HeightFieldLayer(heightField);

    layer->setLocator(locator.get());

    osgTerrain::TerrainTile * tile = new osgTerrain::TerrainTile();
    tile->setName("Terrain tile");
    tile->setElevationLayer(layer.get());
    tile->setTileID(tileID);
    tile->setRequiresNormals(true);
    // set some tile specific uniforms
    osg::ref_ptr<osg::StateSet> tileStateSet = tile->getOrCreateStateSet();
    tileStateSet->addUniform(new osg::Uniform("tileID", tileID.x, tileID.y));   // ivec2
    tileStateSet->addUniform(new osg::Uniform("tileLeftBack", osg::Vec2f(minX, minZ))); // vec2

    return tile;
}

void TerrainGenerator::createBiomes(osgTerrain::TerrainTile & tile, physx::PxHeightFieldSample * pxHeightFieldSamples) const
{
    assert(tile.getNumColorLayers() == 0);

    // use random biome types, 0..biomeTypeCount-1 for now
    const unsigned biomeTypeCount = 4;
    std::uniform_int_distribution<> rndBiomeType(0, biomeTypeCount-1);

    /** physx uses 8 bit for material indices, the highest bit set the orientation of the diagonal.
        So use the absolute value of terrain data in shader. 
        Use material0 only, so setting one terrain type per quad, containing two triangles. */
    // we have to use floats for the sampler, int won't work :(
    typedef float IDTexType;

    /** Create osg image for image layer, storing the terrain id for the graphics. */
    osg::ref_ptr<osg::Image> terrainTypeData = new osg::Image();
    terrainTypeData->allocateImage(m_settings.rows, m_settings.columns, 1, GL_RED, GL_FLOAT);
    assert(terrainTypeData->isDataContiguous());
    assert(terrainTypeData->getTotalDataSize() == m_settings.rows * m_settings.columns * sizeof(IDTexType));
    IDTexType * dataPtr = reinterpret_cast<IDTexType*>(terrainTypeData->data());

    // osg column == physx row
    // osg row == numColumns - physx column - 1 (osg rows going to y, physx rows to z, where osgY == -physxZ)
    const unsigned numOsgRows = m_settings.columns;
    const unsigned numOsgColumns = m_settings.rows;
    for (unsigned physxRow = 0; physxRow < m_settings.rows; ++physxRow) {
        const unsigned physxRowOffset = physxRow * m_settings.columns;
        for (unsigned physxColumn = 0; physxColumn < m_settings.columns; ++physxColumn) {
            unsigned char biomeTypeId = static_cast<unsigned char>(rndBiomeType(rng));
            // scale biome id to 0..1 -> values greater than 1 seem not to work with osg::Image
            IDTexType biomeTypeId_tex = static_cast<IDTexType>(biomeTypeId) / (biomeTypeCount-1);
            // set osg heightfield data, matching the physx rows/columns, both in row major order
            unsigned osgColumn = physxRow;
            unsigned osgRow = numOsgRows - 1 - physxColumn;
            dataPtr[osgColumn + osgRow * numOsgColumns] = biomeTypeId_tex;
            // physx: use default terrain (0), or make a hole
            biomeTypeId = biomeTypeId ?  0 : PxHeightFieldMaterial::eHOLE;
            pxHeightFieldSamples[physxColumn + physxRowOffset].materialIndex0 = biomeTypeId;
            pxHeightFieldSamples[physxColumn + physxRowOffset].materialIndex1 = biomeTypeId;
        }
    }
    

    osg::ref_ptr<osgTerrain::ImageLayer> terrainTypeLayer = new osgTerrain::ImageLayer(terrainTypeData.get());
    terrainTypeLayer->setName(std::string("terrainTypeIDs"));


    tile.setColorLayer(0, terrainTypeLayer.get());

    assert(tile.getNumColorLayers() == 1);
}

ElemateHeightFieldTerrain::ElemateHeightFieldTerrain(const TerrainSettings & settings)
: m_settings(settings)
, m_osgTerrain(nullptr)
, m_osgTerrainTransform(nullptr)
{
}

osg::MatrixTransform * ElemateHeightFieldTerrain::osgTransformedTerrain() const
{
    assert(m_osgTerrainTransform.valid());
    assert(m_osgTerrainTransform->getChildIndex(m_osgTerrain) != m_osgTerrainTransform->getNumChildren());
    return m_osgTerrainTransform.get();
}

osgTerrain::Terrain * ElemateHeightFieldTerrain::osgTerrain() const
{
    assert(m_osgTerrain.valid());
    return m_osgTerrain.get();
}

PxShape const * ElemateHeightFieldTerrain::pxShape(const osgTerrain::TileID & tileID) const
{
    return m_pxShapes.at(tileID);
}

PxRigidStatic * ElemateHeightFieldTerrain::pxActor(const osgTerrain::TileID & tileID) const
{
    return m_pxActors.at(tileID);
}

const std::map<osgTerrain::TileID, physx::PxRigidStatic*> ElemateHeightFieldTerrain::pxActorMap() const
{
    return m_pxActors;
}

const TerrainSettings & ElemateHeightFieldTerrain::settings() const
{
    return m_settings;
}

