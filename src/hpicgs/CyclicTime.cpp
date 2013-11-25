

#include <cmath>

#include "Timer.h"
#include "MathMacros.h"
#include "CyclicTime.h"


const long CyclicTime::utcOffset()
{
	long tz;

#ifdef __GNUC__
	tz = timezone;
#else // __GNUC__
	_get_timezone(&tz);
#endif // __GNUC__

	return tz;
}

// ToDo: C++11 constructor forwarding

CyclicTime::CyclicTime(
	t_longf time
,   t_longf secondsPerCycle)
:   m_timer(new Timer())
,   m_secondsPerCycle(secondsPerCycle)
,   m_mode(TimePausing)
,   m_offset(0.f)
,   m_lastModeChangeTime(0.f)
,   m_utcOffset(0)
{
	initialize();
	setf(time, true);
}

CyclicTime::CyclicTime(
	const time_t & time
,   const time_t & utcOffset
,   t_longf secondsPerCycle)
:   m_timer(new Timer())
,   m_secondsPerCycle(secondsPerCycle)
,   m_mode(TimePausing)
,   m_offset(0.f)
,   m_lastModeChangeTime(0.f)
,   m_utcOffset(utcOffset)
{
    m_timer->setAutoUpdating(true);

	initialize();
	sett(time, true);
}

void CyclicTime::initialize()
{
	m_lastModeChangeTime = m_timer->elapsed();

	m_timef[0] = 0.f;
	m_timef[1] = 0.f;
	m_timef[2] = 0.f;

	m_time[0]  = 0;
	m_time[1]  = 0;
	m_time[2]  = 0;
}

CyclicTime::~CyclicTime()
{
	delete m_timer;
}

const t_longf CyclicTime::getSecondsPerCycle() const
{
    return m_secondsPerCycle;
}

inline long double CyclicTime::elapsed() const
{
    return m_timer->elapsed() * 1e-9f;
}

void CyclicTime::update()
{
    const t_longf elapsed = TimeCycling == m_mode ? this->elapsed() : m_lastModeChangeTime;

    const t_longf elapsedTimef = m_secondsPerCycle > 0.f ? this->elapsed() / m_secondsPerCycle : 0.f;

	m_timef[1] = frac(m_timef[0] + elapsedTimef + m_offset);

	m_time[1] = fToSeconds(elapsedTimef + m_offset) + static_cast<time_t>(m_time[0]);
}

const t_longf CyclicTime::getf(const bool updateFirst)
{
	if (updateFirst)
		update();

	return m_timef[1];
}

const t_longf CyclicTime::getf() const
{
    return m_timef[1];
}

const time_t CyclicTime::gett() const
{
    return m_time[1] + utcOffset();
}

const t_longf CyclicTime::setf(
	t_longf timef
,   const bool forceUpdate)
{
	timef = frac(timef);

	if (timef == 1.f)
		timef = 0.f;

	m_timef[0] = timef;
	m_timef[2] = m_timef[0];

	m_offset = 0;

	const time_t seconds(fToSeconds(timef));

#ifdef __GNUC__
	struct tm lcl(*localtime(&m_time[1]));
#else // __GNUC__
	struct tm lcl;
	localtime_s(&lcl, &m_time[1]);
#endif // __GNUC__      

	lcl.tm_hour = intcast(seconds) / 3600;
	lcl.tm_min  = intcast(seconds) % 3600 / 60;
	lcl.tm_sec  = intcast(seconds) % 60;

	time_t mt = mktime(&lcl);
	if(mt == -1)
		m_time[0] = m_time[2] = 0;
	else
		m_time[0] = m_time[2] = mktime(&lcl) - utcOffset();

	reset(forceUpdate);

	return getf();
}

const t_longf CyclicTime::getNonModf(const bool updateFirst)
{
	return secondsTof(gett(updateFirst));
}

const time_t CyclicTime::gett(const bool updateFirst)
{
	if (updateFirst)
		update();

	return m_time[1] + utcOffset();
}

const time_t CyclicTime::sett(
	const time_t &time
,   const bool forceUpdate)
{
	time_t t = time - utcOffset();

	m_time[0] = t;
	m_time[2] = m_time[0];

	m_timef[0] = frac(secondsTof(t));
	m_timef[2] = m_timef[0];

	m_offset = 0;

	reset(forceUpdate);

	return gett();
}

const t_longf CyclicTime::setSecondsPerCycle(const t_longf secondsPerCycle)
{
	// intepret elapsed seconds within new cycle time
    const t_longf elapsed = TimeCycling == m_mode ? this->elapsed() : m_lastModeChangeTime;

	if(m_secondsPerCycle > 0.f)
		m_offset += elapsed / m_secondsPerCycle;

	m_lastModeChangeTime = 0;

	m_secondsPerCycle = secondsPerCycle;
	m_timer->start();

	return getSecondsPerCycle();
}

inline const t_longf CyclicTime::secondsTof(const time_t &time)
{
	return static_cast<t_longf>((time) / (60.0 * 60.0 * 24.0));
}

inline const time_t CyclicTime::fToSeconds(const t_longf time)
{
	return static_cast<time_t>(time * 60.0 * 60.0 * 24.0 + 0.1);
}

const bool CyclicTime::isRunning() const
{
	return TimeCycling == m_mode;
}

void CyclicTime::start(const bool forceUpdate)
{
	if (m_mode != TimePausing)
		return;

	if (m_secondsPerCycle > 0.f)
		m_offset -= (elapsed() - m_lastModeChangeTime) / m_secondsPerCycle;

	m_mode = TimeCycling;

	if (forceUpdate)
		update();
}

void CyclicTime::pause(const bool forceUpdate)
{
	if (m_mode != TimeCycling)
		return;

	m_lastModeChangeTime = elapsed();

	m_mode = TimePausing;

	if (forceUpdate)
		update();
}

void CyclicTime::reset(const bool forceUpdate)
{
	m_offset = 0.f;
	m_lastModeChangeTime = 0.f;

	m_timef[0] = m_timef[2];
	m_time[0] = m_time[2];

	delete m_timer;
	m_timer = new Timer();

	if (forceUpdate)
		update();
}

void CyclicTime::stop(const bool forceUpdate)
{
	pause();
	reset(forceUpdate);
}

const time_t CyclicTime::getUtcOffset() const
{
	return m_utcOffset;
}

const time_t CyclicTime::setUtcOffset(const time_t &utcOffset)
{
	m_utcOffset = utcOffset;

	return getUtcOffset();
}
