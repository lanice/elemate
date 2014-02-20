#pragma once

#include <memory>
#include <list>

class Achievement;

class AchievementManager
{
public:
    static AchievementManager* instance();

    ~AchievementManager();

    void drawAchievements();
    void resizeAchievements(int width, int height);

protected:
    static std::unique_ptr<AchievementManager> m_instance;
    
    std::list<Achievement> m_locked;
    std::list<Achievement> m_unlocked;

    AchievementManager();

private:
    AchievementManager(const AchievementManager&) = delete;
    void operator=(const AchievementManager&) = delete;
};