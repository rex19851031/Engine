#pragma once

#ifndef CLOCK_HPP
#define CLOCK_HPP

#include <string>
#include <vector>
#include <map>

namespace Henry
{

struct AlarmCallbackArgs
{
	std::string m_rawArgString;
	std::vector<std::string> m_argList;
};

typedef void(AlarmCallbackFunc)(const AlarmCallbackArgs& args);

class Clock;
struct Alarm
{
	Alarm( std::string name , double period , AlarmCallbackFunc func , const std::string& args , bool repeat = false) 
		: m_name(name) 
		, m_period(period) 
		, m_currentTime(0.0)
		, m_repeat(repeat) 
		, m_callbackFunction(func) 
		, m_attachedClock(nullptr)
		, m_destroy(false) { m_args.m_rawArgString = args; };

	std::string m_name;
	double m_period;
	double m_currentTime;
	bool m_repeat;
	bool m_destroy;

	AlarmCallbackFunc* m_callbackFunction;
	AlarmCallbackArgs m_args;
	Clock* m_attachedClock;

	double GerPercentRemaining(){ return m_currentTime / m_period; };
	double GetSecondsRemaining(){ return m_period - m_currentTime; };
	void FireCallbackFunction();
	void AdvanceTime(double deltaSeconds);
};


class Clock
{
public:
	Clock(const std::string& name = "root", Clock* parentClock = nullptr , float timeScale = 1.0f , double maxTimeStep = 1.0 );
	~Clock(void);
	double GetAbsoluteTimeSeconds();
	double GetTime();
	void AdvanceTime(double deltaTime);
	void AppendChild(Clock* clock);
	void RemoveChild(Clock* clock = nullptr);
	void AppendAlarm(Alarm* alarm);
	void RemoveAlarm(const Alarm* _alarm);
	static void RemoveClock(Clock* clock);
	Alarm* FindAlarm(const std::string& name);
	void Dispose(bool appendChildToParent = false);
	void Pause();
	void UnPause();

public:
	static std::map< std::string , Clock* > s_clockMap;
	static Clock* s_rootClock;
	static double s_secondsPerCount;

	std::string m_name;
	std::vector<Clock*> m_childClocks;
	std::vector<Alarm*> m_alarms;
	Clock* m_parent;
	float m_timeScale;
	double m_lastTimeStep;
	double m_maxDeltaTimeStep;
	bool m_pause;

private:
	double m_currentTimeSeconds;
};

}

#endif