#pragma once

#include "terraintile.h"

class WaterTile : public TerrainTile
{
public:
    WaterTile(const TileID & tileID);

protected:
    virtual void initializeProgram() override;

private:
    WaterTile() = delete;
    void operator()(WaterTile&) = delete;
};
