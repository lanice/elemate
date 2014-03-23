#pragma once

#include "glow/ref_ptr.h"
#include "glm/glm.hpp"

#include <list>
#include <unordered_map>
#include <chrono>

namespace glow{
    class VertexArrayObject;
    class Program;
    class Texture;
}

#include <string>

/** @brief Represents a single achievable achievement. */
class Achievement
{
public:
    Achievement(const std::string& title, const std::string& text = "", bool unlocked = false, const std::string& picture = "default");

    /** Specifies property which is needed to unlock the achievement. */
    void setUnlockProperty(const std::string& property_name, const std::string& relation, float property_value);
    /** Locks the achievement. */
    void lock();
    /** Unlocks the achievement. */
    void unlock();
    /** Checks if given unlock-properties are fulfullable. */
    bool unlockable(const std::unordered_map<std::string, float>& properties) const;
    
    bool isUnlocked() const;

    void initialize();
    void update();
    void draw(float x = 0.7f, float y = 0.9f, bool popup = true, float scale = 1.0f);
    bool wasDrawn()const;
    void resize(int width, int height);

    std::string title() const;
    std::string text() const;

    static std::list<std::string> splitText(std::string text, size_t maxLength);
protected:
    static const float ACHIEVEMENT_DISPLAY_TIME;

    float       m_timeMod;
    std::chrono::time_point<std::chrono::system_clock>  m_unlockTime;

    std::string m_title;
    std::string m_text;
    bool        m_unlocked;
    bool        m_drawn;
    std::string m_picture;

    std::unordered_map<std::string, std::pair<std::string, float>> m_properties;

    glow::ref_ptr<glow::VertexArrayObject>  m_vao;
    glow::ref_ptr<glow::Program>            m_program;
    glm::vec2                               m_viewport;
    glow::ref_ptr<glow::Texture>            m_texture;

private:
    Achievement(const Achievement&) = delete;
    void operator=(const Achievement&) = delete;
};