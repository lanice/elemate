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

uint8_t LiquidTile::elementIndexAt(unsigned int row, unsigned int column) const
{
    // hack: see constructor :)
    return valueAt(row, column) > 0.01 ? 1u : 0u;
}

void LiquidTile::setElement(unsigned int /*row*/, unsigned int /*column*/, uint8_t /*elementIndex*/)
{
    glow::warning("setting element type on LiquidTile is not supported.");
    assert(false);
}
