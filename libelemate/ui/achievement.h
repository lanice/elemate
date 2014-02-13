#pragma once

#include "rendering/string_rendering/StringDrawer.h"
#include "glow/ref_ptr.h"
#include "glm/glm.hpp"

#include <list>

#include <chrono>

namespace glow{
	class VertexArrayObject;
	class Program;
}

#include <string>

class Achievement
{
public:
	Achievement(const std::string& title, const std::string& text = "", bool unlocked = false, const std::string& picture = "default");

	void lock();
	void unlock();
	bool isUnlocked() const;

	void initialize();
	void update();
	void draw();
	void resize(int width, int height);

	std::string title() const;
	std::string text() const;

	static std::list<std::string> splitText(std::string text, size_t maxLength);
protected:
	static const float ACHIEVEMENT_DISPLAY_TIME;

	float		m_timeMod;
	std::chrono::time_point<std::chrono::system_clock>	m_unlockTime;

	std::string m_title;
	std::string m_text;
	bool		m_unlocked;
	std::string m_picture;

	StringDrawer m_stringDrawer;

	glow::ref_ptr<glow::VertexArrayObject>	m_vao;
	glow::ref_ptr<glow::Program>			m_program;
	glm::vec2								m_viewport;
	glow::ref_ptr<glow::Texture>			m_texture;

private:
	Achievement(const Achievement&) = delete;
	void operator=(const Achievement&) = delete;
};