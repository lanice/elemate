#pragma once

#include "terraintile.h"

class BaseTile : public TerrainTile
{
public:
    BaseTile(Terrain & terrain, const TileID & tileID);
    virtual ~BaseTile() override;

    virtual void bind(const CameraEx & camera) override;
    virtual void unbind() override;

protected:
    virtual void initialize() override;
    virtual void initializeProgram() override;
    virtual void pxSamplesAndMaterials(physx::PxHeightFieldSample * hfSamples, physx::PxReal heightScale, physx::PxMaterial ** &materials) override;
    virtual void createTerrainTypeTexture();

    virtual physx::PxU8 pxMaterialIndexAt(unsigned int row, unsigned int column) const override;

    glow::ref_ptr<glow::Texture> m_terrainTypeTex;
    glow::ref_ptr<glow::Buffer> m_terrainTypeBuffer;
    glow::UByteArray * m_terrainTypeData;

    glow::ref_ptr<glow::Texture> m_rockTexture;

    friend class TerrainGenerator;

public:
    BaseTile() = delete;
    void operator=(BaseTile&) = delete;
};
