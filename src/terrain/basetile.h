#pragma once

#include "terraintile.h"

class BaseTile : public TerrainTile
{
public:
    BaseTile(Terrain & terrain, const TileID & tileID, const std::initializer_list<std::string> & elementNames);
    virtual ~BaseTile() override;

    virtual void bind(const CameraEx & camera) override;
    virtual void unbind() override;

protected:
    virtual void initialize() override;
    virtual void initializeProgram() override;
    virtual void createTerrainTypeTexture();

    virtual uint8_t elementIndexAt(unsigned int row, unsigned int column) const override;

    /** list of elements this tile consits of. The index of an element in this list equals its index in the terrain type texture.
      * Index 0 is reserved for debug purpose. */
    std::vector<std::string> m_elementNames;

    glow::ref_ptr<glow::Texture> m_terrainTypeTex;
    glow::ref_ptr<glow::Buffer> m_terrainTypeBuffer;
    glow::UByteArray * m_terrainTypeData;
    virtual void updateGlBuffers() override;

    void loadInitTexture(const std::string & elementName, int textureSlot);
    
    typedef std::tuple<std::string, glow::ref_ptr<glow::Texture>, int> TextureTuple;
    std::vector<TextureTuple> m_textures;

    friend class TerrainGenerator;

public:
    BaseTile() = delete;
    void operator=(BaseTile&) = delete;
};
