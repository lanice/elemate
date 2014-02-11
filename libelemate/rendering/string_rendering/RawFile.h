/* This document is based on a file by Max Jendruk (Project Mammut) serving the same purpose but using Qt. */
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>

class RawFile
{
public:
    RawFile(const std::string & filePath);
    virtual ~RawFile();

    const char * data() const;
    const size_t size() const;

    bool isValid() const;
    const std::string & filePath() const;

protected:
    bool readFile();
    void readRawData(std::ifstream & ifs);

    const std::string m_filePath;
    std::vector<char> m_data;

    bool m_valid;
private:
    RawFile(const RawFile&) = delete;
    void operator=(const RawFile&) = delete;
};

