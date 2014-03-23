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

    /** Adds an achievement with tile, substring and picture to the achievementpool. */
    void addAchievement(const std::string& title, const std::string& text = "", const std::string& picture = "default", bool unlocked = false);
    /** Unlocks achievement with name title. */
    void unlockAchievement(const std::string& title);
    
    /** Set property used for achievement unlocking. */
    void setProperty(const std::string& name, float value);
    /** Get current property used for achievement unlocking. */
    float getProperty(const std::string& name) const;

    /** Draws newly unlocked achievements onto the screen. */
    void drawAchievements();
    void resizeAchievements(int width, int height);

    /** Get locked achievements. */
    std::unordered_map<std::string, Achievement*>* getLocked();
    /** Get unlocked achievements. */
    std::unordered_map<std::string, Achievement*>* getUnlocked();

    /** Registers functions of the AchievementManager that can be used within given LuaWrapper instance. */
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

    void checkForNewUnlocks(bool threaded = false);
    void interruptUnlockerThread();

private:
    AchievementManager(const AchievementManager&) = delete;
    void operator=(const AchievementManager&) = delete;
};