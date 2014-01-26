#pragma once

#include "terraintile.h"

class WaterTile : public TerrainTile
{
public:
    WaterTile(Terrain & terrain, const TileID & tileID);

    virtual void bind(const CameraEx & camera) override;
    virtual void unbind() override;

protected:
    virtual void initializeProgram() override;

    virtual uint8_t elementIndexAt(unsigned int row, unsigned int column) const override;

    //! bind the base height tex to discard water fragments below the base level
    glow::ref_ptr<glow::Texture> m_baseHeightTex;

    friend class TerrainGenerator;

public:
    WaterTile() = delete;
    void operator=(WaterTile&) = delete;
};
