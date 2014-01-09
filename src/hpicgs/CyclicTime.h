
#pragma once

typedef long double t_longf;

#include <time.h>

class Timer;

/** CyclicTime manages an osg::Timer and features an interface for floating time
    in the closed interval [0;1] representing a full day and standard c 
    time (time_t) simultaneously. The time updates have to be requested 
    explicitly, thus simplifying usage between multiple recipients.
    The time starts cycling automatically, but can also be paused, stopped, 
    or set to a specific value.
*/

class CyclicTime
{
	enum CyclingStatus
	{
		TimeCycling
    ,   TimePausing
	};

public:
	static long utcOffset();

	CyclicTime(
		t_longf time = 0.0
    ,   t_longf secondsPerCycle = 0.0);
	CyclicTime(
		const time_t & time
    ,   const time_t & utcOffset ///< In seconds (UTC+01:00 is m_utcOffset = +3600).
    ,   t_longf secondsPerCycle = 0.0);

	~CyclicTime();

	/** Increments time appropriate to secondsPerCycle.
    */
	void update();

	// Cycling manipulation - does not tamper the time.

	void start(bool update = false);
	void pause(bool update = false);
	void reset(bool update = false); ///< Resets the time to initial value (secondsPerCycle remain unchanged).
	void stop (bool update = false); ///< Stops and resets the time.

    inline t_longf getSecondsPerCycle() const;

	t_longf setSecondsPerCycle(t_longf secondsPerCycle);


	/** Float time in the intervall [0;1]
    */
    inline t_longf getf() const;

	t_longf getf(bool updateFirst);

	// Sets only time, date remains unchanged.
	t_longf setf(
		t_longf time
    ,   const bool update = false);

	// Elapsed float time from initialized time.
	t_longf getNonModf(bool update = false);

	/** Time in seconds from initial time.
    */
    inline time_t gett() const;

	time_t gett(bool update);
	time_t sett(
		const time_t &time
    ,   const bool update = false);

	time_t getUtcOffset() const;
	time_t setUtcOffset(const time_t & utcOffset /* In Seconds. */);

	// 

	bool isRunning() const;

protected:
	static inline t_longf secondsTof(const time_t & time);
	static inline time_t fToSeconds(t_longf time);

	void initialize();

    t_longf elapsed() const;
protected:
	Timer * m_timer;

	time_t m_utcOffset;

	time_t m_time[3];   // [2] is for stop
	t_longf m_timef[3]; // [2] is for stop

    t_longf m_offset;

	CyclingStatus m_mode;
	t_longf m_lastModeChangeTime;

	t_longf m_secondsPerCycle;
};

// ToDo: C++11 units.. 
const double SECONDS_PER_HOUR = 3600.0;
