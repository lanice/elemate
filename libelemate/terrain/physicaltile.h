#pragma once

#include "terraintile.h"

namespace physx {
    class PxShape;
    class PxRigidStatic;
}

/** Base class for tiles which can be rendered, with a representation as physx shape.
  * Tile values are defined as height values (y coordinate in world coordinate system). */
class PhysicalTile : public TerrainTile
{
public:
    /** @param elementNames list of elements this tile will contain. **/
    PhysicalTile(Terrain & terrain, const TileID & tileID, const std::initializer_list<std::string> & elementNames);

    /** get the name of the element at the row/column position
      * @return a reference to this name from the internal element list */
    const std::string & elementAt(unsigned int row, unsigned int column) const;
    const std::string & elementAt(unsigned int tileValueIndex) const;

    physx::PxShape * pxShape() const;

protected:
    /** list of elements this tile consist of. The index of an element in this list equals its index in the terrain type texture. */
    const std::vector<std::string> m_elementNames;
    /** convenience function to get the tile specific index for an element name */
    virtual uint8_t elementIndex(const std::string & elementName) const;
    /** @return the index this tile internally uses for the element at the row/column position. Parameters must be in range. */
    virtual uint8_t elementIndexAt(unsigned int tileValueIndex) const = 0;

    /** set the internal element index at the row/column position corresponding to the element name */
    void setElement(unsigned int row, unsigned int column, const std::string & elementName);
    /** set the internal element index at the row/column position to elementIndex.  */
    void setElement(unsigned int row, unsigned int column, uint8_t elementIndex);
    virtual void setElement(unsigned int tileValueIndex, uint8_t elementIndex) = 0;

    virtual void initialize() override;


    virtual void createPxObjects(physx::PxRigidStatic & pxActor);

    physx::PxShape * m_pxShape;

    virtual void createTerrainTypeTexture();
    glow::ref_ptr<glow::Texture> m_terrainTypeTex;
    glow::ref_ptr<glow::Buffer> m_terrainTypeBuffer;
    std::vector<uint8_t> m_terrainTypeData;

    virtual void updateBuffers() override;

    void updatePxHeight();
    void addToPxUpdateBox(unsigned int minRow, unsigned int maxRow, unsigned int minColumn, unsigned int maxColumn);
    struct UIntBoundingBox {
        UIntBoundingBox();
        unsigned int minRow; unsigned int maxRow; unsigned int minColumn; unsigned int maxColumn;
    };
    UIntBoundingBox m_pxUpdateBox;
    void clearPxBufferUpdateRange();

    friend class TerrainInteraction;
    friend class TemperatureTile;
};
