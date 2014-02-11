#include "StringComposer.h"

#include <glow/logging.h>
#include <fstream>

StringComposer::StringComposer()
{
}

StringComposer::~StringComposer()
{
    for (auto pair : m_characterSpecifics)
        delete pair.second;
    m_characterSpecifics.clear();
}

std::list<CharacterSpecifics *> StringComposer::characterSequence(const std::string & string) const
{
    std::list<CharacterSpecifics *> characterSequence;
    
    for (size_t i = 0; i < string.length(); i++) {
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
    
    glow::debug() << "Reading info of font" << findFontName(line);

    getline(file, line); //Additional line ...

    while (getline(file, line))
        parseCharacterLine(line, textureSize);
    
    file.close();
    
    return true;
}

void StringComposer::parseCharacterLine(const std::string & line, float textureSize)
{
    std::list<std::string> parsedValues;
    parseValues(line, parsedValues);
    auto i = parsedValues.begin();

    unsigned int id = std::stoi(*(i++));
    
    float x = std::stof(*(i++));
    float y = std::stof(*(i++));
    float width = std::stof(*(i++));
    float height = std::stof(*(i++));
    float xOffset = std::stof(*(i++));
    float yOffset = std::stof(*(i++));
    
    float xAdvance = std::stof(*(i++)) / textureSize;
    
    glm::vec2 position = glm::vec2(x, textureSize - (y + height)) / textureSize;
    glm::vec2 size = glm::vec2(width, height) / textureSize;
    glm::vec2 offset = glm::vec2(xOffset, yOffset - height) / textureSize;
    
    auto specifics = new CharacterSpecifics { position, size, offset, xAdvance };
    m_characterSpecifics.emplace(id, specifics);
}

size_t StringComposer::parseValues(const std::string& line, std::list<std::string>& resultBuffer)
{
    resultBuffer.clear();

    bool valid = false;
    std::string nextValue;
    for (unsigned int i = 0; i < line.size(); i++)
    {
        if (line[i] == '=')
        {
            nextValue = "";
            valid = true;
        }
        else if (line[i] == ' ')
        {
            if (nextValue.size() > 0 && valid){
                resultBuffer.push_back(nextValue);
                nextValue.clear();
                valid = false;
            }
        }
        else
        {
            nextValue += line[i];
        }
    }
    if (nextValue.size() > 0 && valid){
        resultBuffer.push_back(nextValue);
        nextValue.clear();
        valid = false;
    }
    return resultBuffer.size();
}

std::string StringComposer::findFontName(const std::string& line)
{
    std::string nextValue;
    std::size_t found = line.find("\"");
    if (found != std::string::npos)
        return line.substr(found);
    return "";
}