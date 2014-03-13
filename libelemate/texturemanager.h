#pragma once

#include <string>
#include <unordered_map>

#include <glow/global.h>

class TextureManager
{
public:
    static void initialize();
    static void release();

    /** reserve a new texture unit mapped to the owner and name */
    static int reserveTextureUnit(const std::string & owner, const std::string & name);
    /** get an already reserve texture unit, mapped to the owner and name */
    static int getTextureUnit(const std::string & owner, const std::string & name);

private:
    TextureManager();
    static TextureManager * s_instance;

    int m_reserveTextureUnit(const std::string & owner, const std::string & name);
    int m_getTextureUnit(const std::string & owner, const std::string & name) const;

    int m_nextFreeUnit;
    GLint m_maxUnits;
    /** mapping from texture owner to the named textures, from textures to the reserved texture unit. */
    std::unordered_map<std::string, std::unordered_map<std::string, GLenum>> m_assignedTextures;
};
