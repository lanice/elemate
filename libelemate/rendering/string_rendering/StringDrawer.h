/* This document is based on a file by Max Jendruk (Project Mammut) serving the same purpose but using Qt. */

#pragma once

#include <list>
#include <string>

#include <glm/glm.hpp>
#include <glow/ref_ptr.h>

#include "CharacterDrawable.h"
#include "StringComposer.h"

namespace glow
{
    class Program;
    class Texture;
}

struct TextObject{
    std::string text;
    float x, y, z, scale;
    float red, green, blue;
};

class StringDrawer
{
public:
    enum Alignment { kAlignLeft, kAlignCenter, kAlignRight };
    
    StringDrawer();
    ~StringDrawer();

    bool initialize();
    void paint(const std::string & text,
               const glm::mat4 & modelMatrix,
               Alignment alignment = kAlignLeft,
               const glm::vec3 color = glm::vec3(1.0f));
    void paint(const TextObject& textObject);
    void resize(int width, int height);

    float scaleToWidth(const std::string& text, float maxWidth);

protected:
    bool initializeProgram();
    bool initializeTexture();
    
    glm::mat4 alignmentTransform(const std::list<CharacterSpecifics *> & list,
                                 Alignment alignment) const;

protected:
    static const float s_textureSize;
    
    glow::ref_ptr<glow::Program> m_program;
    glow::ref_ptr<glow::Texture> m_characterAtlas;
    glm::vec2                    m_viewport;

    CharacterDrawable m_drawable;
    StringComposer m_stringComposer;

};
