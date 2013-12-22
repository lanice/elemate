#pragma once

#include "terraintile.h"

class WaterTile : public TerrainTile
{
public:
    WaterTile(const TileID & tileID);

    virtual void bind(const glowutils::Camera & camera) override;
    virtual void unbind() override;

protected:
    virtual void initializeProgram() override;

private:
    WaterTile() = delete;
    void operator()(WaterTile&) = delete;
};
