#include "StringComposer.h"

#include <glow/logging.h>
#include <regex>
#include <fstream>

StringComposer::StringComposer()
{
}

StringComposer::~StringComposer()
{
    for each (auto pair in m_characterSpecifics)
        delete pair.second;
    m_characterSpecifics.clear();
}

std::list<CharacterSpecifics *> StringComposer::characterSequence(const std::string & string) const
{
    std::list<CharacterSpecifics *> characterSequence;
    
    for (int i = 0; i < string.length(); i++) {
        unsigned char id = string[i];
        
        if (m_characterSpecifics.end() == m_characterSpecifics.find(id))
            continue;
        
        characterSequence.push_back(m_characterSpecifics.at(static_cast<unsigned int>(id)));
    }
    
    return characterSequence;
}

bool StringComposer::readSpecificsFromFile(const std::string & fileName, float textureSize)
{
    m_characterSpecifics.clear();
    
    std::ifstream file(fileName);
    
    if (!file.good()) {
        glow::debug() << "Could not open " << fileName;
        return false;
    }
    
    std::string line;
    getline(file, line);
    
    std::regex font_regex("\"(.+)\"");
    auto words_begin = std::sregex_iterator(line.begin(), line.end(), font_regex);
    std::sregex_iterator i = words_begin;

    glow::debug() << "Reading info of font" << (i++)->str();

    getline(file, line); //Additional line ...

    while (getline(file, line))
        parseCharacterLine(line, textureSize);
    
    file.close();
    
    return true;
}

void StringComposer::parseCharacterLine(const std::string & line, float textureSize)
{
    std::regex words_regex("=[^\\s]+");
    auto words_begin = std::sregex_iterator(line.begin(), line.end(), words_regex);
    auto words_end = std::sregex_iterator();
    std::sregex_iterator i = words_begin;

    unsigned int id = std::stoi((i++)->str().substr(1));
    
    float x = std::stof((i++)->str().substr(1));
    float y = std::stof((i++)->str().substr(1));
    float width = std::stof((i++)->str().substr(1));
    float height = std::stof((i++)->str().substr(1));
    float xOffset = std::stof((i++)->str().substr(1));
    float yOffset = std::stof((i++)->str().substr(1));
    
    float xAdvance = std::stof((i++)->str().substr(1)) / textureSize;
    
    glm::vec2 position = glm::vec2(x, textureSize - (y + height)) / textureSize;
    glm::vec2 size = glm::vec2(width, height) / textureSize;
    glm::vec2 offset = glm::vec2(xOffset, yOffset - height) / textureSize;
    
    auto specifics = new CharacterSpecifics { position, size, offset, xAdvance };
    m_characterSpecifics.emplace(id, specifics);
}
