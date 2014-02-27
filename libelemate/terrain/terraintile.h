#pragma once

#include <forward_list>
#include <cstdint>
#include <tuple>
#include <vector>

#include <glow/global.h>
#include <glow/ref_ptr.h>

#include <foundation/PxSimpleTypes.h>

#include <glm/glm.hpp>

#include "terrainsettings.h"

namespace glow {
    class Program;
    class Texture;
    class Buffer;
}
namespace physx {
    class PxShape;
    class PxRigidStatic;
    struct PxHeightFieldSample;
    class PxMaterial;
}
class Terrain;
class CameraEx;

class TerrainTile {
public:
    /** @param terrain registers tile at this terrain
      * @param tileID register tile at this position in the terrain
      * @param elementNames list of elements this tile will contain.
      * @param resolutionScaling use a lower resolution than the maximum defined in the terrain settings.
               a value lower than 1 will result in resolution scaling. */
    TerrainTile(Terrain & terrain, const TileID & tileID, const std::initializer_list<std::string> & elementNames, float resolutionScaling = 1.0f);
    virtual ~TerrainTile();

    /** get the name of the element at the row/column position
      * @return a reference to this name from the internal element list */
    const std::string & elementAt(unsigned int row, unsigned int column) const;

    /** update opengl buffers etc */
    virtual void prepareDraw();

    virtual void bind(const CameraEx & camera);
    virtual void unbind();

    physx::PxShape * pxShape() const;

    /** @return interpolated height (y value) at specified normalized in tile position. */
    float interpolatedHeightAt(float normX, float normZ) const;

    const glm::mat4 & transform() const;

    const uint32_t samplesPerAxis;
    const float resolutionScaling;
    /** number of samples per world coordinate */
    const float samplesPerWorldCoord;
    /** distance between two sample points along one axis */
    const float sampleInterval;


    friend class TerrainGenerator;
    friend class TerrainInteraction;
    friend class Terrain;

protected:
    const TileID m_tileID;

    const Terrain & m_terrain;

    /** list of elements this tile consist of. The index of an element in this list equals its index in the terrain type texture. */
    const std::vector<std::string> m_elementNames;
    /** convenience function to get the tile specific index for an element name */
    virtual uint8_t elementIndex(const std::string & elementName) const = 0;

    /** @return the index this tile internally uses for the element at the row/column position. Parameters must be in range. */
    virtual uint8_t elementIndexAt(unsigned int row, unsigned int column) const = 0;

    virtual void initialize();
    bool m_isInitialized;
    /** subclass has to override this method to create the program.
      * Afterward, call this function to set some uniforms. */
    virtual void initializeProgram() = 0;
    virtual void createPxObjects(physx::PxRigidStatic & pxActor);
    void pxSamplesAndMaterials(
        physx::PxHeightFieldSample * hfSamples,
        physx::PxReal heightScale,
        physx::PxMaterial ** const &materials);

    glow::ref_ptr<glow::Texture> m_heightTex;
    glow::ref_ptr<glow::Buffer>  m_heightBuffer;
    glow::ref_ptr<glow::Program> m_program;

    /** Contains the height field values in row major order.
      * Initially created by the TerrainGenerator. */
    std::vector<float> m_heightField;

    physx::PxShape * m_pxShape;

    glm::mat4 m_transform;


protected: // interaction specific functions (see class TerrainInteraction)
    /** @return height at specified row/column position. Parameters must be in range. */
    float heightAt(unsigned int row, unsigned int column) const;
    /** @param height value at specified row/column position, if values are in range
      * @return whether parameters are in range */
    bool heightAt(unsigned int row, unsigned int column, float & height) const;
    /** set height at specified row/column position. Parameters must be in range. */
    void setHeight(unsigned int row, unsigned int column, float value);
    /** set the internal element index at the row/column position corresponding to the element name */
    virtual void setElement(unsigned int row, unsigned int column, const std::string & elementName);
    /** set the internal element index at the row/column position to elementIndex.  */
    virtual void setElement(unsigned int row, unsigned int column, uint8_t elementIndex) = 0;

    virtual void updateBuffers();

    struct UpdateRange {
        unsigned int startIndex;
        unsigned int nbElements;
    };

    void addBufferUpdateRange(unsigned int startIndex, unsigned int nbElements);
    void clearBufferUpdateRange();
    glm::detail::tvec2<unsigned int> m_updateRangeMinMaxIndex;
    std::forward_list<UpdateRange> m_bufferUpdateList;

    void updatePxHeight();
    void addToPxUpdateBox(unsigned int minRow, unsigned int maxRow, unsigned int minColumn, unsigned int maxColumn);
    struct UIntBoundingBox {
        UIntBoundingBox();
        unsigned int minRow; unsigned int maxRow; unsigned int minColumn; unsigned int maxColumn;
    };
    UIntBoundingBox m_pxUpdateBox;

public:
    TerrainTile() = delete;
    void operator=(TerrainTile&) = delete;
};
