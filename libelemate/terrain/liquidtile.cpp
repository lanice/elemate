#include "liquidtile.h"

#include <glow/Shader.h>
#include <glow/Program.h>
#include <glow/Texture.h>
#include <glowutils/global.h>

#include "terrain.h"
#include "world.h"
#include "texturemanager.h"

LiquidTile::LiquidTile(Terrain & terrain, const TileID & tileID)
: PhysicalTile(terrain, tileID, {"water", "lava"})
{
}

uint8_t LiquidTile::elementIndexAt(unsigned int tileValueIndex) const
{
    // hack: see constructor :)
    assert(tileValueIndex < samplesPerAxis * samplesPerAxis);
    return m_values.at(tileValueIndex) > 0.01 ? 1u : 0u;
}

void LiquidTile::setElement(unsigned int /*row*/, uint8_t /*elementIndex*/)
{
    glow::warning("setting element type on LiquidTile is not supported.");
    assert(false);
}
