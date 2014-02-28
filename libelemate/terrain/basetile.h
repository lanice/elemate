#pragma once

#include "physicaltile.h"

#include <vector>

class BaseTile : public PhysicalTile
{
public:
    BaseTile(Terrain & terrain, const TileID & tileID, const std::initializer_list<std::string> & elementNames);

    virtual void bind(const CameraEx & camera) override;
    virtual void unbind() override;

protected:
    virtual void initialize() override;
    virtual void initializeProgram() override;
    virtual void createTerrainTypeTexture();

    virtual uint8_t elementIndexAt(unsigned int row, unsigned int column) const override;
    /** convenience function to get the tile specific index for an element name */
    virtual uint8_t elementIndex(const std::string & elementName) const override;

    /** set the internal element index at the row/column position to elementIndex.  */
    virtual void setElement(unsigned int row, unsigned int column, uint8_t elementIndex) override;

    glow::ref_ptr<glow::Texture> m_terrainTypeTex;
    glow::ref_ptr<glow::Buffer> m_terrainTypeBuffer;
    std::vector<uint8_t> m_terrainTypeData;
    virtual void updateBuffers() override;

    void loadInitTexture(const std::string & elementName, int textureSlot);
    
    typedef std::tuple<std::string, glow::ref_ptr<glow::Texture>, int> TextureTuple;
    std::vector<TextureTuple> m_textures;

    friend class TerrainGenerator;

public:
    BaseTile() = delete;
    void operator=(BaseTile&) = delete;
};
