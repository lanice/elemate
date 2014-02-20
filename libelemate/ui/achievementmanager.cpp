#include "achievementmanager.h"

#include <glow/logging.h>

#include <ui/achievement.h>
#include <lua/luawrapper.h>

std::unique_ptr<AchievementManager> AchievementManager::m_instance;

AchievementManager::AchievementManager()
{
}

AchievementManager::~AchievementManager()
{
    for (auto& achievement : m_unlocked)
        delete achievement.second;
    for (auto& achievement : m_locked)
        delete achievement.second;
    m_unlocked.clear();
    m_locked.clear();
}

AchievementManager* AchievementManager::instance()
{
    if (!m_instance.get())
        m_instance.reset(new AchievementManager());
    return m_instance.get();
}

void AchievementManager::addAchievement(const std::string& title, const std::string& text, const std::string& picture, bool unlocked)
{
    if (unlocked)
        m_unlocked.emplace(title, new Achievement(title, text, true, picture));
    else
        m_locked.emplace(title, new Achievement(title, text, false, picture));
}

void AchievementManager::unlockAchievement(const std::string& title)
{
    auto iter_found = m_locked.find(title);
    if (iter_found != m_locked.end() && m_unlocked.end() == m_unlocked.find(title) && !iter_found->second->isUnlocked())
    {
        auto result = iter_found;
        result->second->unlock();
        m_unlocked.insert(result, ++iter_found);
        m_locked.erase(result);
    }
    else 
    {
        glow::debug() << "Tried to unlock " << title << ", but achievement was not found!\n";
    }
}

void AchievementManager::drawAchievements()
{
    while (!m_unlocked.empty() && m_unlocked.begin()->second->wasDrawn())
        m_unlocked.erase(m_unlocked.begin());
    if (!m_unlocked.empty())
        m_unlocked.begin()->second->draw();
}

void AchievementManager::resizeAchievements(int width, int height)
{
    for (auto& achievement : m_unlocked)
        achievement.second->resize(width, height);
    for (auto& achievement : m_locked)
        achievement.second->resize(width, height);
}

void AchievementManager::registerLuaFunctions(LuaWrapper * lua)
{
    std::function<int(std::string)> unlock = [=](std::string title)
    { 
        unlockAchievement(title);
        return 0; 
    };

    std::function<int(std::string, std::string, std::string)> add = [=](std::string title, std::string text, std::string picture)
    {   
        addAchievement(title, text, picture);
        return 0;
    };

    lua->Register("achievement_unlock", unlock);
    lua->Register("achievement_add", add);

    lua->loadScript("scripts/achievements.lua");
}
