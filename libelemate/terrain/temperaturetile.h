#pragma once

#include "terraintile.h"

class BaseTile;

typedef float °Celsius;
typedef float meter;

class TemperatureTile : public TerrainTile
{
public:
    TemperatureTile(Terrain & terrain, const TileID & tileId, const BaseTile & m_baseTile);

    const static °Celsius minTemperature;
    const static °Celsius maxTemperature;

    °Celsius temperatureByHeight(meter height);

    virtual void updatePhysics(float delta) override;

protected:
    const BaseTile & m_baseTile;

    float m_deltaTime;

public:
    void operator=(TemperatureTile&) = delete;
};
