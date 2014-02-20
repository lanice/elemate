#include "achievementmanager.h"

#include <string>

#include <ui/achievement.h>

std::unique_ptr<AchievementManager> AchievementManager::m_instance;

AchievementManager::AchievementManager()
{
    m_unlocked.clear();
    m_unlocked.emplace_front(std::string("Raise your hand"), std::string("Lift the hand in the air like you don't care..."));
    m_unlocked.front().unlock();
    m_unlocked.emplace_front(std::string("Baby steps"), std::string("OK, you opened the game. What now?"));
    m_unlocked.front().unlock();
}

AchievementManager::~AchievementManager()
{
    m_unlocked.clear();
    m_locked.clear();
}

AchievementManager* AchievementManager::instance()
{
    if (!m_instance.get())
        m_instance.reset(new AchievementManager());
    return m_instance.get();
}

void AchievementManager::drawAchievements()
{
    while (!m_unlocked.empty() && m_unlocked.front().wasDrawn())
        m_unlocked.pop_front();
    if (!m_unlocked.empty())
        m_unlocked.front().draw();
}

void AchievementManager::resizeAchievements(int width, int height)
{
    for (auto& achievement : m_unlocked)
        achievement.resize(width, height);
    for (auto& achievement : m_locked)
        achievement.resize(width, height);
}