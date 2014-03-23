#include "imagereader.h"

#include <fstream>

#include <glow/logging.h>

RawImage::RawImage(uint32_t width, uint32_t height)
: m_data(nullptr)
, m_size(width * height * 3)
, m_width(width)
, m_height(height)
{
    if (m_size > 0)
        m_data = new char[m_size];
}

RawImage::~RawImage()
{
    delete[] m_data;
}

RawImage::RawImage(const std::string & filename, uint32_t width, uint32_t height)
: RawImage(width, height)
{
    std::ifstream file(filename, std::ios_base::binary);

    if (!file.good()) {
        glow::critical ("RawImage: could not open file %;", filename);
        m_status = Status::InvalidFile;
        return;
    }

    try {
        file.read(m_data, m_size);
    }
    catch (std::ios_base::failure & /*e*/) {
        glow::critical("RawImage: could not read from file %;", filename);
        m_status = Status::ReadError;
        return;
    }

    if (static_cast<uint32_t>(file.gcount()) < m_size) {
        glow::warning("RawImage: file too small: %;\n\t is %; expected: %;", filename, file.gcount(), m_size);
        m_status = Status::InvalidFileSize;
        return;
    }

    if (static_cast<uint32_t>(file.gcount()) > m_size) {
        glow::warning("RawImage: file too large: %;\n\t is %; expected: %;", filename, file.gcount(), m_size);
        m_status = Status::InvalidFileSize;
        return;
    }

    m_status = Status::Success;
}

char * RawImage::rawData()
{
    return m_data;
}

uint32_t RawImage::width() const
{
    return m_width;
}

uint32_t RawImage::height() const
{
    return m_height;
}

RawImage::Status RawImage::status() const
{
    return m_status;
}
