#pragma once

#include "terraintile.h"

class BaseTile : public TerrainTile
{
public:
    BaseTile(Terrain & terrain, const TileID & tileID);
    virtual ~BaseTile() override;

    virtual void bind(const glowutils::Camera & camera) override;
    virtual void unbind() override;

protected:
    virtual void initializeProgram() override;
    virtual void pxSamplesAndMaterials(physx::PxHeightFieldSample * hfSamples, physx::PxReal heightScale, physx::PxMaterial ** &materials) override;
    virtual void createTerrainTypeTexture();

    glow::ref_ptr<glow::Texture> m_terrainTypeTex;
    glow::ref_ptr<glow::Buffer> m_terrainTypeBuffer;
    glow::UByteArray * m_terrainTypeData;

    friend class TerrainGenerator;

private:
    BaseTile() = delete;
    void operator()(BaseTile&) = delete;
};
