#pragma once

#include <glow/ref_ptr.h>
#include <glow/Array.h>

#include "terrainsettings.h"

namespace glow {
    class VertexArrayObject;
    class Buffer;
    class Program;
    class Texture;
}
namespace glowutils {
    class Camera;
}
namespace physx {
    class PxShape;
}
class Terrain;

class TerrainTile {
public:
    TerrainTile(Terrain & terrain, const TileID & tileID);
    virtual ~TerrainTile();

    virtual void bind(const glowutils::Camera & camera);
    virtual void unbind();

    physx::PxShape * pxShape() const;

protected:
    const TileID m_tileID;

    const Terrain * m_terrain;

    virtual void initialize();
    virtual void initializeProgram() = 0;

    glow::ref_ptr<glow::Texture> m_heightTex;
    glow::ref_ptr<glow::Program> m_program;

    glow::FloatArray * m_heightField;

    physx::PxShape * m_pxShape;

    glm::mat4 m_transform;

    friend class TerrainGenerator;

private:
    TerrainTile() = delete;
    void operator=(TerrainTile&) = delete;
};
