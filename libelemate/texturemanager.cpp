#include "texturemanager.h"

#include <cassert>

#include <glow/logging.h>

TextureManager * TextureManager::s_instance = nullptr;

using namespace std;

void TextureManager::initialize()
{
    assert(s_instance == nullptr);
    s_instance = new TextureManager();
}

void TextureManager::release()
{
    assert(s_instance);
    delete s_instance;
    s_instance = nullptr;
}

TextureManager::TextureManager()
: m_nextFreeUnit(1)
{
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &m_maxUnits);
    CheckGLError();
}

int TextureManager::reserveTextureUnit(const string & owner, const string & name)
{
    assert(s_instance);
    return s_instance->m_reserveTextureUnit(owner, name);
}

int TextureManager::getTextureUnit(const string & owner, const string & name)
{
    assert(s_instance);
    return s_instance->m_getTextureUnit(owner, name);
}

int TextureManager::m_reserveTextureUnit(const string & owner, const string & name)
{
    int unit = m_nextFreeUnit++;
    assert(unit < m_maxUnits);
    if (unit >= m_maxUnits) {
        glow::fatal("Requesting more texture units than available: current hardware/driver is limited to %; units", m_maxUnits);
        return 0;
    }

    assert(m_assignedTextures[owner].find(name) == m_assignedTextures[owner].end());
    if (m_assignedTextures[owner].find(name) != m_assignedTextures[owner].end()) {
        glow::warning("Requesting a texture for %;:%;, but this name is already in use. Overwriting the old assignment..", owner, name);
    }
    m_assignedTextures[owner][name] = unit;

    return unit;
}

int TextureManager::m_getTextureUnit(const string & owner, const string & name) const
{
    auto ownerMapIt = m_assignedTextures.find(owner);
    assert(ownerMapIt != m_assignedTextures.end());
    if (ownerMapIt == m_assignedTextures.end()) {
        glow::fatal("Asking for the texture unit for %;:%;, but this wasn't reserved before.");
        return 0;
    }

    auto unitIt = ownerMapIt->second.find(name);
    if (unitIt == ownerMapIt->second.end()) {
        glow::fatal("Asking for the texture unit for %;:%;, but this wasn't reserved before.");
        return 0;
    }

    return unitIt->second;
}
