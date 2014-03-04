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
    virtual void initializeProgram();
    glow::ref_ptr<glow::Program> m_program;

    virtual uint8_t elementIndexAt(unsigned int row, unsigned int column) const override;

    /** set the internal element index at the row/column position to elementIndex.  */
    virtual void setElement(unsigned int row, unsigned int column, uint8_t elementIndex) override;

    void loadInitTexture(const std::string & elementName, int textureSlot);
    
    typedef std::tuple<std::string, glow::ref_ptr<glow::Texture>, int> TextureTuple;
    std::vector<TextureTuple> m_textures;

    friend class TerrainGenerator;

public:
    BaseTile() = delete;
    void operator=(BaseTile&) = delete;
};
