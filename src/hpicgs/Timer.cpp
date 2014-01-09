
#include "ChronoTimer.h"
#include "Timer.h"


Timer::Timer(
    bool start
,   bool autoUpdate)
:   m_chrono(new ChronoTimer(start, autoUpdate))
{
}

Timer::~Timer()
{
    delete m_chrono;
}

void Timer::update() const
{
    m_chrono->update();
}

bool Timer::paused() const
{
    return m_chrono->paused();
}

void Timer::start()
{
    m_chrono->start();
}

void Timer::pause()
{
    m_chrono->pause();
}

void Timer::stop()
{
    m_chrono->stop();
}

void Timer::reset()
{
    m_chrono->reset();
}

long double Timer::elapsed() const
{
    return m_chrono->elapsed();
}

void Timer::setAutoUpdating(const bool auto_update)
{
    m_chrono->setAutoUpdating(auto_update);
}

bool Timer::autoUpdating() const
{
    return m_chrono->autoUpdating();
}
