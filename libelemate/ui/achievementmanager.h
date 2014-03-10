#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <thread>

#include <rendering/string_rendering/StringDrawer.h>

class Achievement;
class LuaWrapper;

class AchievementManager
{
public:
    static AchievementManager* instance();

    ~AchievementManager();

    void addAchievement(const std::string& title, const std::string& text = "", const std::string& picture = "default", bool unlocked = false);
    void unlockAchievement(const std::string& title);
    
    /** Properties needed for complex and time-critic Achievements */
    void  setProperty(const std::string& name, float value);
    float getProperty(const std::string& name) const;
    void  checkForNewUnlocks(bool threaded = false);


    void drawAchievements();
    void resizeAchievements(int width, int height);

    void registerLuaFunctions(LuaWrapper * lua);

protected:
    static std::unique_ptr<AchievementManager> m_instance;
    
    std::unordered_map<std::string, float>  m_properties;
    std::unique_ptr<std::thread>            m_unlockerThread;
    bool                                    m_unlockerThreadRunning;

    std::unordered_map<std::string, Achievement*> m_locked;
    std::unordered_map<std::string, Achievement*> m_drawQueue;
    std::unordered_map<std::string, Achievement*> m_unlocked;

    LuaWrapper*  m_lua;

    AchievementManager();
    void interruptUnlockerThread();

private:
    AchievementManager(const AchievementManager&) = delete;
    void operator=(const AchievementManager&) = delete;
};