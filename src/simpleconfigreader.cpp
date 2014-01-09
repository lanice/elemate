/** \file SimpleConfigReader.h
 * Contains the definitions for the SimpleConfigReader class.
 * \author  Friedrich Horschig
 * \date    November 11, 2013
 * \version 1.0
 */

#include "simpleconfigreader.h"
#include <fstream>

SimpleConfigReader::SimpleConfigReader():
    m_values(),
    m_filename(""),
    m_auto_update(true)
{}

SimpleConfigReader::SimpleConfigReader(const std::string& filename):
    m_values(),
    m_filename(filename),
    m_auto_update(true)
{
    readConfig();
}

SimpleConfigReader::~SimpleConfigReader()
{
}

bool SimpleConfigReader::readConfig()
{
    if (m_filename == "")
        return false;
    std::ifstream file(m_filename);
    std::string line;
    try
    {
        while(getline(file, line)){
            auto pos = line.find("=");
            setValue(line.substr(0,pos),line.substr(pos+1));
        }
    }
    catch (int /*error_number*/)
    {
        return false;
    }
    file.close();
    return true;
}

bool SimpleConfigReader::writeConfig()
{
    if (m_filename == "")
        return false;
    std::ofstream file(m_filename);
    try
    {
        for(const auto& pair : m_values){
            file << pair.first << "=" << pair.second << std::endl;
        }
    }
    catch(int /*error_number*/)
    {
        return false;
    }
    file.close();
    return true;
}

bool SimpleConfigReader::readConfig(const std::string& filename)
{
    m_filename = filename;
    return readConfig();
}

bool SimpleConfigReader::writeConfig(const std::string& filename)
{
    m_filename = filename;
    return writeConfig();
}

void SimpleConfigReader::setAutoUpdate(bool enable_auto_update)
{
    m_auto_update = enable_auto_update;
}

void SimpleConfigReader::setValue(const std::string& key, const std::string& value)
{
    if(key.find("=") == std::string::npos && value != "")
        m_values[key] = value;
}

std::string  SimpleConfigReader::value(std::string key) const
{
    auto iterator = m_values.find(key); 
    if (iterator != m_values.end())
        return iterator->second;
    else
        return "";
}

bool SimpleConfigReader::auto_update() const
{
    return m_auto_update;
}