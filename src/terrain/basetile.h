#pragma once

#include "terraintile.h"

namespace physx {
    struct PxHeightFieldSample;
}

class BaseTile : public TerrainTile
{
public:
    BaseTile(Terrain & terrain, const TileID & tileID);
    virtual ~BaseTile() override;

    virtual void bind(const glowutils::Camera & camera) override;
    virtual void unbind() override;

protected:
    virtual void initializeProgram() override;
    virtual void createTerrainTypeTexture(const physx::PxHeightFieldSample * pxHeightFieldSamples);

    glow::ref_ptr<glow::Texture> m_terrainTypeTex;
    glow::UByteArray * m_terrainTypeData;

    friend class TerrainGenerator;

private:
    BaseTile() = delete;
    void operator()(BaseTile&) = delete;
};
