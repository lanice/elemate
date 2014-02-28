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
    class Program;
    class Texture;
    class Buffer;
}
class Terrain;
class CameraEx;

class TerrainTile {
public:
    /** @param terrain registers tile at this terrain
      * @param tileID register tile at this position in the terrain
      * @param resolutionScaling use a lower resolution than the maximum defined in the terrain settings.
               a value lower than 1 will result in resolution scaling. */
    TerrainTile(Terrain & terrain, const TileID & tileID, float resolutionScaling = 1.0f);
    virtual ~TerrainTile();

    /** update opengl buffers etc */
    virtual void prepareDraw();

    virtual void bind(const CameraEx & camera);
    virtual void unbind();

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

    virtual void initialize();
    bool m_isInitialized;

    glow::ref_ptr<glow::Texture> m_heightTex;
    glow::ref_ptr<glow::Buffer>  m_heightBuffer;

    /** Contains the height field values in row major order.
      * Initially created by the TerrainGenerator. */
    std::vector<float> m_heightField;

    glm::mat4 m_transform;


protected: // interaction specific functions (see class TerrainInteraction)
    /** @return height at specified row/column position. Parameters must be in range. */
    float heightAt(unsigned int row, unsigned int column) const;
    /** @param height value at specified row/column position, if values are in range
      * @return whether parameters are in range */
    bool heightAt(unsigned int row, unsigned int column, float & height) const;
    /** set height at specified row/column position. Parameters must be in range. */
    void setHeight(unsigned int row, unsigned int column, float value);

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
