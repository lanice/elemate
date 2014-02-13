#include "achievement.h"

#include <glow/Buffer.h>
#include <glow/VertexArrayObject.h>
#include <glow/VertexAttributeBinding.h>
#include <glow/Program.h>
#include <glow/Texture.h>
#include <glow/logging.h>

#include <glowutils/global.h>

#include "io/imagereader.h"

Achievement::Achievement(const std::string& title, const std::string& text, bool unlocked, const std::string& picture) :
m_title(title)
, m_text(text)
, m_unlocked(unlocked)
, m_picture(picture)
{
	initialize();
}

void Achievement::initialize()
{
	const int TEXTURE_SIZE = 256;
	
	m_stringDrawer.initialize();
	std::vector<glm::vec2> points({
		glm::vec2(+1.f, -1.f)
		, glm::vec2(+1.f, +1.f)
		, glm::vec2(-1.f, -1.f)
		, glm::vec2(-1.f, +1.f)
	});

	glow::Buffer* vbo = new glow::Buffer(GL_ARRAY_BUFFER);
	vbo->setData(points, GL_STATIC_DRAW);

	m_vao = new glow::VertexArrayObject();

	m_vao->bind();

	glow::VertexAttributeBinding * vertexBinding = m_vao->binding(0);

	vertexBinding->setAttribute(0);
	vertexBinding->setBuffer(vbo, 0, sizeof(glm::vec2));
	vertexBinding->setFormat(2, GL_FLOAT, GL_FALSE, 0);
	m_vao->enable(0);

	m_vao->unbind();

	m_program = new glow::Program();
	m_program->attach(
		glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/ui/achievement.vert"),
		glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/ui/achievement.frag"));

	m_program->setUniform("picture", 0);

	m_texture = new glow::Texture(GL_TEXTURE_2D);
	m_texture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_texture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_texture->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_texture->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

	RawImage image("data/textures/achievements/" + m_picture + ".raw", TEXTURE_SIZE, TEXTURE_SIZE);

	m_texture->bind();
	m_texture->image2D(0, GL_RGB8, TEXTURE_SIZE, TEXTURE_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, image.rawData());
	m_texture->unbind();
}

void Achievement::draw(float time_left)
{
	m_texture->bindActive(GL_TEXTURE0);

	m_program->setUniform("time_left", time_left);
	m_program->setUniform("ratio", m_viewport.x / m_viewport.y);

	m_program->use();
	m_vao->bind();

	m_vao->drawArrays(GL_TRIANGLE_STRIP, 0, 4);

	m_vao->unbind();
	m_program->release();

	m_texture->unbindActive(GL_TEXTURE0);

	m_stringDrawer.paint(m_title,
		glm::mat4(0.5, 0, 0, 0,
		0, 0.5, 0, 0,
		0, 0, 0.5, 0,
		0.7, 0.9, 0, 1));

	std::list<std::string> split;
	std::string text(m_text);
	while (!text.empty())
	{
		if (text.length() > 24)
		{
			split.push_back(text.substr(0, 24));
			text = text.substr(24);
		}		
		else
		{
			split.push_back(text);
			text = "";
		}
	}

	float pos = 0.85f;
	for (auto& line : split)
	{
		m_stringDrawer.paint(line,
			glm::mat4(0.25, 0, 0, 0,
			0, 0.25, 0, 0,
			0, 0, 0.25, 0,
			0.7, pos, 0, 1));
		pos -= 0.05f;
	}
}

void Achievement::lock()
{
	m_unlocked = false;
}

void Achievement::unlock()
{
	m_unlocked = true;
}

bool Achievement::isUnlocked() const
{
	return m_unlocked;
}

std::string Achievement::title() const
{
	return m_title;
}

std::string Achievement::text() const
{
	return m_text;
}

void Achievement::resize(int width, int height)
{
	m_viewport = glm::vec2(width, height);
	m_stringDrawer.resize(width, height);
}