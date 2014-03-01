#pragma once

#include "terraintile.h"

class BaseTile;

typedef float celsius;
typedef float meter;

class TemperatureTile : public TerrainTile
{
public:
    TemperatureTile(Terrain & terrain, const TileID & tileId, const BaseTile & m_baseTile);

    const static celsius minTemperature;
    const static celsius maxTemperature;

    celsius temperatureByHeight(meter height);

    virtual void updatePhysics(float delta) override;

protected:
    const BaseTile & m_baseTile;

    float m_deltaTime;

public:
    void operator=(TemperatureTile&) = delete;
};
