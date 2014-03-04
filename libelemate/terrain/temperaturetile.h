#pragma once

#include "terraintile.h"

class PhysicalTile;

typedef float celsius;
typedef float meter;

class TemperatureTile : public TerrainTile
{
public:
    TemperatureTile(Terrain & terrain, const TileID & tileId, PhysicalTile & baseTile, PhysicalTile & liquidTile);

    const static celsius minTemperature;
    const static celsius maxTemperature;
    const static celsius minLavaTemperature;

    celsius temperatureByHeight(meter height);

    virtual void updatePhysics(double delta) override;

protected:
    PhysicalTile & m_baseTile;
    PhysicalTile & m_liquidTile;

    double m_deltaTime;

public:
    void operator=(TemperatureTile&) = delete;
};
