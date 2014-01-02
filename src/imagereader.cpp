#include "imagereader.h"

#include <algorithm>

#include <png.h>

RawImage::RawImage(size_t size /*= 0*/)
: m_data(nullptr)
, m_size(size)
{
    if (size > 0)
        m_data = new unsigned char[size];
}

RawImage::~RawImage()
{
    delete[] m_data;
}

RawImage::RawImage(const std::string & filename)
: RawImage()
{
    size_t dot = filename.find_last_of(".");
    if (dot == std::string::npos) {
        m_status = Status::UnknownImageType;
        return;
    }
    std::string ext = filename.substr(dot, filename.size() - dot);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == "png")
        readPngFromFile(filename);
    else {
        m_status = Status::UnknownImageType;
        return;
    }
}

bool RawImage::readPngFromFile(const std::string & filename)
{
    png_image image;

    memset(&image, 0, sizeof(image));
    image.version = PNG_IMAGE_VERSION;

    if (!png_image_begin_read_from_file(&image, filename.c_str())) {
        m_status = Status::InvalidFile;
        return false;
    }

    image.format = PNG_FORMAT_RGB;

    m_size = PNG_IMAGE_SIZE(image);
    m_data = new uint8_t[m_size];

    png_bytep buffer = reinterpret_cast<png_bytep>(m_data);

    if (buffer == NULL) {
        // running out of memory while allocating image
        m_status = Status::OutOfMemory;
        return false;
    }

    if (!png_image_finish_read(&image, NULL, buffer, 0, NULL)) {
        // oom while reading png. This is the only place where we need to clean up the image manually
        png_image_free(&image);
        m_status = Status::OutOfMemory;
        return false;
    }

    m_width = image.width;
    m_height = image.height;

    m_status = Status::Success;

    return true;
}

RawImage::Status RawImage::status() const
{
    return m_status;
}

uint8_t * RawImage::data()
{
    return m_data;
}

uint8_t * const RawImage::data() const
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
