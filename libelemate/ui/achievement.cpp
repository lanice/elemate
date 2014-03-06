#include "achievement.h"

#include <glow/Buffer.h>
#include <glow/VertexArrayObject.h>
#include <glow/VertexAttributeBinding.h>
#include <glow/Program.h>
#include <glow/Texture.h>
#include <glow/logging.h>

#include <glowutils/global.h>

#include "io/imagereader.h"
#include "rendering/string_rendering/StringDrawer.h"


const float Achievement::ACHIEVEMENT_DISPLAY_TIME = 1;

Achievement::Achievement(const std::string& title, const std::string& text, bool unlocked, const std::string& picture) :
m_title(title)
, m_text(text)
, m_unlocked(unlocked)
, m_picture(picture)
, m_drawn(false)
, m_timeMod(0)
{
    initialize();
}

void Achievement::initialize()
{
    const int TEXTURE_SIZE = 256;
    
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

void Achievement::draw(StringDrawer& stringDrawer)
{
    if (!m_unlocked)
        return;

    update();
    m_texture->bindActive(GL_TEXTURE0);

    m_program->setUniform("time_mod", m_timeMod);
    m_program->setUniform("ratio", m_viewport.x / m_viewport.y);

    m_program->use();
    m_vao->bind();

    m_vao->drawArrays(GL_TRIANGLE_STRIP, 0, 4);

    m_vao->unbind();
    m_program->release();

    m_texture->unbindActive(GL_TEXTURE0);
    float pos = 0.9f + m_timeMod;
    float scale = stringDrawer.scaleToWidth(m_title, 0.25f);
    stringDrawer.paint(m_title,
        glm::mat4(  scale, 0, 0, 0,
                    0, scale, 0, 0,
                    0, 0, scale, 0,
                    0.7, pos, 0, 1));
    pos -= 0.06f;

    scale = stringDrawer.scaleToWidth(m_text, 0.25f);
    glow::debug() << scale;
    for (auto& line : splitText(m_text, 5/scale))
    {
        stringDrawer.paint(line,
            glm::mat4(  scale, 0, 0, 0,
                        0, scale, 0, 0,
                        0, 0, scale, 0,
                        0.7, pos, 0, 1));
        pos -= 0.05f;
    }
}

std::list<std::string> Achievement::splitText(std::string text, size_t maxLength)
{
    std::list<std::string> split;
    size_t pos = 0;
    size_t last_split_pos = 0;
    while (!text.empty()){
        while (pos < maxLength && pos <= text.length()){
            if (text[pos] == ' ' || text[pos] == '\n' || text[pos] == '-' || text[pos] == '\0')
                last_split_pos = pos;
            if (text[pos] == '\n')
                break;
            pos++;
        }
        if (!last_split_pos)
            last_split_pos = maxLength;
        if (text.length() > last_split_pos) {
            std::string nextLine(text.substr(0, last_split_pos));
            if (' ' == nextLine[0] || '\n' == nextLine[0])
                nextLine = nextLine.substr(1);
            split.push_back(nextLine);
            text = text.substr(last_split_pos);
            pos = 0;
        }
        else {
            split.push_back(text);
            text = "";
        }
    }
    return split;
}

void Achievement::update(){
    if (!m_timeMod)
        m_unlockTime = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = std::chrono::system_clock::now() - m_unlockTime;
    m_timeMod = static_cast<float>(1.0 - diff.count()/ACHIEVEMENT_DISPLAY_TIME);
    m_timeMod *= m_timeMod;
    for (int i = 0; i < ACHIEVEMENT_DISPLAY_TIME;i++)
        m_timeMod *= m_timeMod*m_timeMod;
    if (diff.count() > 2*ACHIEVEMENT_DISPLAY_TIME)
    {
        m_drawn = true;
    }
}

void Achievement::lock()
{
    m_unlocked  = false;
    m_drawn     = false;
    m_timeMod   = 0;
}

void Achievement::setUnlockProperty(const std::string& property_name, const std::string& relation, float property_value)
{
    m_properties.emplace(property_name, std::make_pair(relation, property_value));
}

bool Achievement::unlockable(const std::unordered_map<std::string, float>& properties) const
{
    float current_value = 0, goal_value = 0;
    std::string relation = "";
    bool fulfilled = true;

    for (auto& prop : m_properties)
    {
        auto iter = properties.find(prop.first);
        if (iter != properties.end())
        {
            current_value = properties.at(prop.first);
        }
        else
        {
            glow::debug() << "An achievement-property seems to have no initial value!";
            return false;
        }

        relation        = prop.second.first; 
        goal_value      = prop.second.second;
        
        fulfilled = fulfilled &&
            (relation == "<" && current_value < goal_value) ||
            (relation == ">" && current_value > goal_value) ||
            ((relation == ">=" || relation == "=>") && current_value >= goal_value) ||
            ((relation == "<=" || relation == "=<") && current_value <= goal_value) ||
            ((relation == "="  || relation == "==") && current_value == goal_value) ||
            ((relation == "<>" || relation == "!=") && current_value != goal_value);
        if (!fulfilled)
            break;
    }
    return fulfilled;
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
}

bool Achievement::wasDrawn() const
{
    return m_drawn;
}