#pragma once

#include <unordered_map>
#include <string>
#include <thread>

#include <rendering/string_rendering/StringDrawer.h>

class Achievement;
class LuaWrapper;

class AchievementManager
{
public:
    static void initialize();
    static void release();
    static AchievementManager* instance();

    void addAchievement(const std::string& title, const std::string& text = "", const std::string& picture = "default", bool unlocked = false);
    void unlockAchievement(const std::string& title);
    
    /** Properties needed for complex and time-critic Achievements */
    void  setProperty(const std::string& name, float value);
    float getProperty(const std::string& name) const;
    void  checkForNewUnlocks(bool threaded = false);


    void drawAchievements();
    void resizeAchievements(int width, int height);

    std::unordered_map<std::string, Achievement*>* getLocked();
    std::unordered_map<std::string, Achievement*>* getUnlocked();

    void registerLuaFunctions(LuaWrapper * lua);

protected:
    AchievementManager();
    ~AchievementManager();

    static AchievementManager * m_instance;
    
    std::unordered_map<std::string, float>  m_properties;
    std::thread *                           m_unlockerThread;
    bool                                    m_stopUnlockerThread;

    std::unordered_map<std::string, Achievement*> m_locked;
    std::unordered_map<std::string, Achievement*> m_drawQueue;
    std::unordered_map<std::string, Achievement*> m_unlocked;

    void interruptUnlockerThread();

private:
    AchievementManager(const AchievementManager&) = delete;
    void operator=(const AchievementManager&) = delete;
};