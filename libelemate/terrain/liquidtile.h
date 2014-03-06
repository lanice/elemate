#pragma once

#include "physicaltile.h"

class LiquidTile : public PhysicalTile
{
public:
    LiquidTile(Terrain & terrain, const TileID & tileID);

protected:
    virtual uint8_t elementIndexAt(unsigned int tileValueIndex) const override;

    /** no effect for this kind of tile */
    virtual void setElement(unsigned int tileValueIndex, uint8_t elementIndex) override;

    friend class TerrainGenerator;

public:
    LiquidTile() = delete;
    void operator=(LiquidTile&) = delete;
};
