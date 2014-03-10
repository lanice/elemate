#include "achievementmanager.h"

#include <glow/logging.h>

#include <ui/achievement.h>
#include <lua/luawrapper.h>

std::unique_ptr<AchievementManager> AchievementManager::m_instance;

AchievementManager::AchievementManager()
{
    checkForNewUnlocks(true);
}

AchievementManager::~AchievementManager()
{
    interruptUnlockerThread();
    for (auto& achievement : m_unlocked)
        delete achievement.second;
    for (auto& achievement : m_locked)
        delete achievement.second;
    for (auto& achievement : m_drawQueue)
        delete achievement.second;
    m_unlocked.clear();
    m_drawQueue.clear();
    m_locked.clear();
}

AchievementManager* AchievementManager::instance()
{
    if (!m_instance.get()){
        m_instance.reset(new AchievementManager());
    }
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
    if (iter_found != m_locked.end() && m_drawQueue.end() == m_drawQueue.find(title) && !iter_found->second->isUnlocked())
    {
        auto result = iter_found;
        result->second->unlock();
        m_drawQueue.insert(result, ++iter_found);
        m_locked.erase(result);
    }
}

void AchievementManager::drawAchievements()
{
    while (!m_drawQueue.empty() && m_drawQueue.begin()->second->wasDrawn()){
        auto first_achievement = m_drawQueue.begin();
        m_unlocked.insert(m_drawQueue.begin(), ++first_achievement);
        m_drawQueue.erase(m_drawQueue.begin());
    }
    if (!m_drawQueue.empty())
        m_drawQueue.begin()->second->draw();
}

void AchievementManager::resizeAchievements(int width, int height)
{
    for (auto& achievement : m_unlocked)
        achievement.second->resize(width, height);
    for (auto& achievement : m_locked)
        achievement.second->resize(width, height);
    StringDrawer::instance()->resize(width, height);
}

void AchievementManager::registerLuaFunctions(LuaWrapper * lua)
{
    m_lua = lua;
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
    
    std::function<int(std::string, std::string, std::string, float)> condition = [=](std::string title, std::string property_name, std::string relation, float value)
    {
        m_locked.at(title)->setUnlockProperty(property_name, relation, value);
        return 0;
    };

    std::function<int(std::string, float)> setProperty = [=](std::string property_name, float property_value)
    {
        AchievementManager::setProperty(property_name, property_value);
        return 0;
    };

    std::function<float(std::string)> getProperty = [=](std::string property_name)
    {
        return m_properties.at(property_name);
    };
    
    m_lua->Register("achievement_unlock", unlock);
    m_lua->Register("achievement_add", add);
    m_lua->Register("achievement_condition", condition);
    m_lua->Register("achievement_setProperty", setProperty);
    m_lua->Register("achievement_getProperty", getProperty);

    m_lua->loadScript("scripts/achievements.lua");
}

void  AchievementManager::setProperty(const std::string& name, float value)
{
    auto prop = m_properties.find(name);
    if (prop != m_properties.end())
        prop->second = value;
    else
        m_properties.emplace(name, value);
}

float AchievementManager::getProperty(const std::string& name) const
{
    return m_properties.at(name);
}


std::unordered_map<std::string, Achievement*>* AchievementManager::getLocked()
{
    return &m_locked;
}

std::unordered_map<std::string, Achievement*>* AchievementManager::getUnlocked()
{
    return &m_unlocked;
}

void  AchievementManager::checkForNewUnlocks(bool threaded)
{
    if (threaded)
    {
        interruptUnlockerThread();
        m_unlockerThread.reset(new std::thread(&AchievementManager::checkForNewUnlocks, this, false));
        return;
    }
    m_unlockerThreadRunning = true;
    std::list<std::string> new_unlocks;
    while (m_unlockerThreadRunning)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(2500));
        new_unlocks.clear();
        for (auto& achievement : m_locked)
            if (achievement.second->unlockable(m_properties))
                new_unlocks.push_back(achievement.first);
            for (const auto& unlocked_achievement : new_unlocks)
                unlockAchievement(unlocked_achievement);
    }
    m_unlockerThreadRunning = false;
}

void AchievementManager::interruptUnlockerThread()
{
    if (!(m_unlockerThread.get() && m_unlockerThreadRunning))
        return;
    m_unlockerThreadRunning = false;
    m_unlockerThread->detach();
}