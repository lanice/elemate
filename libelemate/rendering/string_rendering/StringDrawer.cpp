#include "StringDrawer.h"

#include <numeric>

#include <glow/logging.h>
#include <fstream>

#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <glow/Program.h>
#include <glow/Shader.h>
#include <glow/Texture.h>
#include <glowutils/global.h>

#include "RawFile.h"
#include "texturemanager.h"


const float StringDrawer::s_textureSize = 1024.0f;

StringDrawer::StringDrawer()
{

}

StringDrawer::~StringDrawer()
{

}

bool StringDrawer::initialize()
{
    if (!initializeTexture())
        return false;

    if (!initializeProgram())
        return false;
    
    if (!m_stringComposer.readSpecificsFromFile("data/font/P22UndergroundPro-Medium.txt", s_textureSize))
        return false;
    
    m_drawable.initialize();

    return true;
}

bool StringDrawer::initializeProgram()
{
    m_program = new glow::Program();

    m_program->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/utils/string_drawer.vert"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/utils/string_drawer.frag"));

    m_program->setUniform("characterAtlas", TextureManager::getTextureUnit("StringDrawer", "characterAtlas"));

    return true;
}

bool StringDrawer::initializeTexture()
{
    m_characterAtlas = new glow::Texture();
    
    const std::string fileName("data/font/P22UndergroundPro-Medium.1024.1024.r.ub.raw");
    
    RawFile file(fileName);
    
    if (!file.isValid())
        return false;
    
    m_characterAtlas->image2D(  0, 
                                GL_R8, 
                                static_cast<GLsizei>(s_textureSize), 
                                static_cast<GLsizei>(s_textureSize),
                                0, 
                                GL_RED, 
                                GL_UNSIGNED_BYTE, 
                                file.data());
    
    m_characterAtlas->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_characterAtlas->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    m_characterAtlas->bindActive(GL_TEXTURE0 + TextureManager::reserveTextureUnit("StringDrawer", "characterAtlas"));
    glActiveTexture(GL_TEXTURE0);
    CheckGLError();

    return true;
}

void StringDrawer::paint(
    const std::string & text,
    const glm::mat4 & modelMatrix,
    Alignment alignment, 
    const glm::vec3 color)
{
    m_program->setUniform("color", color);
    
    std::list<CharacterSpecifics *> list = m_stringComposer.characterSequence(text);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    
    glm::mat4 transform = modelMatrix * alignmentTransform(list, alignment);
    glm::vec2 aspectRatio(1,m_viewport.x/ m_viewport.y);

    for (auto* currentSpecifics : list)
    {
        glm::mat4 positionTransform;
        glm::mat4 textureCoordTransform;

        textureCoordTransform *= glm::translate(glm::vec3(currentSpecifics->position, 0.0f));
        textureCoordTransform *= glm::scale(glm::vec3(currentSpecifics->size, 1.0f));
        
        positionTransform *= transform;
        positionTransform *= glm::translate(glm::vec3(currentSpecifics->offset, 0.0f));
        positionTransform *= glm::scale(glm::vec3(currentSpecifics->size*aspectRatio, 1.0f));
        
        m_program->setUniform("positionTransform", positionTransform);
        m_program->setUniform("textureCoordTransform", textureCoordTransform);
        
        m_program->use();
        m_drawable.draw();
        m_program->release();
        
        transform *= glm::translate(currentSpecifics->xAdvance, 0.0f, 0.0f);
    }
    
    glDisable(GL_BLEND);
}

void StringDrawer::paint(const TextObject& textObject)
{
    paint(textObject.text, 
        glm::mat4(  textObject.scale, 0, 0, 0,
                    0, textObject.scale, 0, 0,
                    0, 0, textObject.scale, 0,
                    textObject.x, textObject.y, textObject.z, 1),
                    StringDrawer::Alignment::kAlignLeft,
                    glm::vec3(textObject.red, textObject.green, textObject.blue));
}

glm::mat4 StringDrawer::alignmentTransform(const std::list<CharacterSpecifics *> & list, Alignment alignment) const
{
    float offset=0.0F;
    
    const float length = std::accumulate(list.begin(), list.end(), 0.0f,
        [] (float sum, CharacterSpecifics * specifics) {
            return sum + specifics->xAdvance;
        });
    
    switch (alignment) {
        case kAlignLeft:
            offset = - list.front()->offset.x;
            break;
            
        case kAlignCenter:
            offset = - length / 2.0f;
            break;
            
        case kAlignRight:
            offset = - length;
            break;
    }
    
    return glm::translate(offset, 0.0f, 0.0f);
}

void StringDrawer::resize(int width, int height)
{
    m_viewport = glm::vec2(width, height);
}