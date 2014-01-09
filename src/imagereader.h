#pragma once

#include <cstdint>
#include <string>

class RawImage
{
public:
    RawImage(uint32_t width, uint32_t height);
    RawImage(const std::string & filename, uint32_t width, uint32_t height);
    ~RawImage();

    enum class Status {
        Success,
        InvalidFile,
        InvalidFileSize,
        ReadError
    };

    Status status() const;
    char * rawData();
    /** @return allocated size in bytes */
    size_t size() const;

    uint32_t width() const;
    uint32_t height() const;

protected:
    Status m_status;

    char * m_data;
    size_t m_size;
    uint32_t m_width;
    uint32_t m_height;
};
