#pragma once

#include <forward_list>
#include <cstdint>
#include <tuple>
#include <vector>

#include <glow/global.h>
#include <glow/ref_ptr.h>

#include <glm/glm.hpp>

#include "terrainsettings.h"

namespace glow {
    class Texture;
    class Buffer;
}
class Terrain;
class CameraEx;


/** @brief Abstract base class for Terrain tiles.

    This is basically a quadratic grid which is provided as a OpenGL texture buffer. 
    This class implements interpolated access to the floating point values and lazy buffer updates as needed.
    The tile will register itself in the passed Terrain instance. */
class TerrainTile
{
public:
    /** @param terrain registers tile at this terrain
      * @param tileID register tile at this position in the terrain */
    TerrainTile(Terrain & terrain, const TileID & tileID, float minValidValue, float maxValidValue, float interactStdDeviation);
    virtual ~TerrainTile();

    const std::string tileName;

    /** update OpenGL buffers etc */
    virtual void prepareDraw();

    virtual void bind(const CameraEx & camera);
    virtual void unbind();

    /** timed update function that subclasses can implement */
    virtual void updatePhysics(double delta);

    /** @return interpolated value at specified normalized in tile position. */
    float interpolatedValueAt(float normX, float normZ) const;

    const glm::mat4 & transform() const;

    const uint32_t samplesPerAxis;
    const float resolutionScaling;
    /** number of samples per world coordinate */
    const float samplesPerWorldCoord;
    /** distance between two sample points along one axis */
    const float sampleInterval;

    /** @return tile value at specified row/column position. Parameters must be in range. */
    float valueAt(unsigned int row, unsigned int column) const;
    float valueAt(unsigned int index) const;
    /** @param tile value at specified row/column position, if values are in range
    * @return whether parameters are in range */
    bool valueAt(unsigned int row, unsigned int column, float & value) const;
    /** set tile value at specified row/column position. Parameters must be in range. */
    void setValue(unsigned int row, unsigned int column, float value);
    void setValue(unsigned int index, float value);

    inline bool isValueInRange(float value) const {
        return (value + 0.001f) >= minValidValue && value <= maxValidValue;
    }

    const float minValidValue;
    const float maxValidValue;
    const float interactStdDeviation;   // used in the TerrainInteraction to scale the interaction radius


    friend class TerrainGenerator;
    friend class TerrainInteraction;
    friend class Terrain;

protected:
    const TileID m_tileID;

    static std::string generateName(const TileID & tileID);

    const Terrain & m_terrain;

    virtual void initialize();
    bool m_isInitialized;

    glow::ref_ptr<glow::Texture> m_valueTex;
    glow::ref_ptr<glow::Buffer>  m_valueBuffer;

    /** Contains the tile values in row major order. */
    std::vector<float> m_values;

    glm::mat4 m_transform;

    bool m_drawHeatMap;


protected:
    virtual void updateBuffers();

    struct UpdateRange {
        unsigned int startIndex;
        unsigned int nbElements;
    };

    void addBufferUpdateRange(unsigned int startIndex, unsigned int nbElements);
    glm::detail::tvec2<unsigned int> m_updateRangeMinMaxIndex;
    std::forward_list<UpdateRange> m_bufferUpdateList;

private:
    void clearBufferUpdateRange();

public:
    TerrainTile() = delete;
    void operator=(TerrainTile&) = delete;
};
