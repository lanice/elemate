#pragma once

#include <forward_list>

#include <glow/ref_ptr.h>
#include <glow/Array.h>

#include <foundation/PxSimpleTypes.h>

#include "terrainsettings.h"

namespace glow {
    class Program;
    class Texture;
    class Buffer;
}
namespace glowutils {
    class Camera;
}
namespace physx {
    class PxShape;
    class PxRigidStatic;
    struct PxHeightFieldSample;
    class PxMaterial;
}
class Terrain;

class TerrainTile {
public:
    /** @param terrain registers tile at this terrain
      * @param tileID register tile at this position in the terrain */
    TerrainTile(Terrain & terrain, const TileID & tileID);
    virtual ~TerrainTile();

    virtual void bind(const glowutils::Camera & camera);
    virtual void unbind();

    physx::PxShape * pxShape() const;

    /** @return interpolated height (y value) at specified normalized in tile position. */
    float interpolatedHeightAt(float normX, float normZ) const;

    friend class TerrainGenerator;
    friend class TerrainInteractor;

protected:
    const TileID m_tileID;

    const Terrain & m_terrain;

    virtual void initialize();
    bool isInitialized;
    /** subclass has to override this method to create the program.
      * Afterward, call this function to set some uniforms. */
    virtual void initializeProgram() = 0;
    virtual void createPxObjects(physx::PxRigidStatic & pxActor);
    virtual void pxSamplesAndMaterials(
        physx::PxHeightFieldSample * hfSamples,
        physx::PxReal heightScale,
        physx::PxMaterial ** &materials) = 0;

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

    /** @return physx material index at specified row/column position. Parameters must be in range. */
    virtual physx::PxU8 pxMaterialIndexAt(unsigned int row, unsigned int column) const = 0;

    void addBufferUpdateRange(GLintptr offset, GLsizeiptr length);
    std::forward_list<std::pair<GLintptr, GLsizeiptr>> m_bufferUpdateList;
    virtual void updateGlBuffers();

public:
    TerrainTile() = delete;
    void operator=(TerrainTile&) = delete;
};
