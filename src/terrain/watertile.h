#pragma once

#include "terraintile.h"

class WaterTile : public TerrainTile
{
public:
    WaterTile(Terrain & terrain, const TileID & tileID);

    virtual void bind(const glowutils::Camera & camera) override;
    virtual void unbind() override;

protected:
    virtual void initializeProgram() override;
    virtual void pxSamplesAndMaterials(physx::PxHeightFieldSample * hfSamples, physx::PxReal heightScale, physx::PxMaterial ** &materials) override;

    virtual physx::PxU8 pxMaterialIndexAt(unsigned int row, unsigned int column) const override;

private:
    WaterTile() = delete;
    void operator()(WaterTile&) = delete;
};
