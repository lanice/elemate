#pragma once

#include "terraintile.h"

class BaseTile : public TerrainTile
{
public:
    BaseTile(const TileID & tileID);

protected:
    virtual void initializeProgram() override;

private:
    BaseTile() = delete;
    void operator()(BaseTile&) = delete;
};
