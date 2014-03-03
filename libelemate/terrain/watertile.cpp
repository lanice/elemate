#include "watertile.h"

#include <glow/Shader.h>
#include <glow/Program.h>
#include <glow/Texture.h>
#include <glowutils/global.h>

#include "terrain.h"
#include "world.h"
#include "texturemanager.h"

WaterTile::WaterTile(Terrain & terrain, const TileID & tileID)
: PhysicalTile(terrain, tileID, {"water"})
{
}

void WaterTile::bind(const CameraEx & camera)
{
    PhysicalTile::bind(camera);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
}

void WaterTile::unbind()
{
    glDisable(GL_BLEND);

    PhysicalTile::unbind();
}

uint8_t WaterTile::elementIndexAt(unsigned int /*row*/, unsigned int /*column*/) const
{
    return 0u;  // currently water only, no data for different elements
}

uint8_t WaterTile::elementIndex(const std::string & elementName) const
{
    if (elementName != "water")
        glow::warning("Trying to fetch an element other than water from water terrain level.");
    assert(elementName == "water");
    return 0u;
}

void WaterTile::setElement(unsigned int /*row*/, unsigned int /*column*/, uint8_t /*elementIndex*/)
{
    glow::warning("setting element type on WaterTile is not supported.");
    assert(false);
}

