#pragma once

#include <forward_list>
#include <cstdint>

#include <glow/ref_ptr.h>
#include <glow/Array.h>

#include <foundation/PxSimpleTypes.h>

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
      * @param elementNames list of elements this tile will contain. */
    TerrainTile(Terrain & terrain, const TileID & tileID, const std::initializer_list<std::string> & elementNames);
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

    glm::mat4 transform() const;

    friend class TerrainGenerator;
    friend class TerrainInteractor;
    friend class Terrain;

protected:
    const TileID m_tileID;

    const Terrain & m_terrain;

    /** list of elements this tile consits of. The index of an element in this list equals its index in the terrain type texture. */
    const std::vector<std::string> m_elementNames;

    /** @return the index this tile internaly uses for the element at the row/column position. Parameters must be in range. */
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
    glow::FloatArray * m_heightField;
    /** Used by terrain generator to set the height field.
      * Allocates gpu memory and initializes the associated texture. */
    void setHeightField(glow::FloatArray & heightField);

    physx::PxShape * m_pxShape;

    glm::mat4 m_transform;


protected: // interaction specific functions (see class TerrainInteractor)
    /** @return height at specified row/column position. Parameters must be in range. */
    float heightAt(unsigned int row, unsigned int column) const;
    /** @param height value at specified row/column position, if values are in range
      * @return whether parameters are in range */
    bool heightAt(unsigned int row, unsigned int column, float & height) const;
    /** set height at specified row/column position. Parameters must be in range. */
    void setHeight(unsigned int row, unsigned int column, float value);

    void addBufferUpdateRange(GLintptr offset, GLsizeiptr length);
    std::forward_list<std::pair<GLintptr, GLsizeiptr>> m_bufferUpdateList;
    virtual void updateGlBuffers();

public:
    TerrainTile() = delete;
    void operator=(TerrainTile&) = delete;
};
