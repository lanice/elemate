#pragma once

#include <memory>
#include <unordered_map>
#include <string>

class Achievement;
class LuaWrapper;

class AchievementManager
{
public:
    static AchievementManager* instance();

    ~AchievementManager();

    void addAchievement(const std::string& title, const std::string& text = "", const std::string& picture = "default", bool unlocked = false);
    void unlockAchievement(const std::string& title);
    
    void drawAchievements();
    void resizeAchievements(int width, int height);

    void registerLuaFunctions(LuaWrapper * lua);

protected:
    static std::unique_ptr<AchievementManager> m_instance;
    
    std::unordered_map<std::string, Achievement*> m_locked;
    std::unordered_map<std::string, Achievement*> m_unlocked;

    AchievementManager();

private:
    AchievementManager(const AchievementManager&) = delete;
    void operator=(const AchievementManager&) = delete;
};