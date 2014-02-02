/* This document is based on a file by Max Jendruk (Project Mammut) serving the same purpose but using Qt. */
#pragma once

#include <list>
#include <unordered_map>
#include <string>

#include <glm/glm.hpp>

struct CharacterSpecifics
{
    glm::vec2 position;
    glm::vec2 size;
    glm::vec2 offset;
    float xAdvance;
};

class StringComposer
{
public:
    StringComposer();
    ~StringComposer();

    bool readSpecificsFromFile(const std::string & fileName, float textureSize);
    std::list<CharacterSpecifics *> characterSequence(const std::string & string) const;

protected:
    void parseCharacterLine(const std::string & line, float textureSize);
    size_t parseValues(const std::string& line, std::list<std::string>& resultBuffer);
    std::string findFontName(const std::string& line);

    std::unordered_map<unsigned int, CharacterSpecifics *> m_characterSpecifics;
};
