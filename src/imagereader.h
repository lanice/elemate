#pragma once

#include <cstdint>
#include <string>

class RawImage
{
public:
    RawImage(size_t size = 0);
    RawImage(const std::string & filename);
    ~RawImage();

    enum class Status {
        Success,
        InvalidFile,
        UnknownImageType,
        OutOfMemory
    };

    bool readPngFromFile(const std::string & filename);

    Status status() const;
    uint8_t * data();
    uint8_t * const data() const;
    size_t size() const;

    uint32_t width() const;
    uint32_t height() const;

protected:
    Status m_status;

    uint8_t * m_data;
    size_t m_size;
    uint32_t m_width;
    uint32_t m_height;
};
