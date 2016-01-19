#include "Clock.hpp"

#include "Engine\Core\HenryFunctions.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <algorithm>
#include <string> 
#include <iostream>


namespace Henry
{

std::map< std::string , Clock* > Clock::s_clockMap;
Clock* Clock::s_rootClock = nullptr;
double Clock::s_secondsPerCount;

double IntializeTimeSystem(LARGE_INTEGER& out_initialTime)
{
	LARGE_INTEGER countsPerSecond;
	QueryPerformanceFrequency( &countsPerSecond );
	QueryPerformanceCounter( &out_initialTime );
	return( 1.0 / static_cast< double >( countsPerSecond.QuadPart ) );
}


Clock::Clock(const std::string& name , Clock* parentClock , float timeScale , double maxTimeStep) 
	: m_name(name)
	, m_parent(parentClock)
	, m_timeScale(timeScale)
	, m_lastTimeStep(0.0)
	, m_pause(false)
	, m_maxDeltaTimeStep(maxTimeStep)
	, m_currentTimeSeconds(0.0)
{
	if(!m_parent)
	{
		if(s_clockMap.size() == 0 && !s_rootClock)
		{
			s_rootClock = this;
			m_parent = nullptr;
		}
		else
		{
			m_parent = s_rootClock;
			m_parent->AppendChild(this);
		}
	}
	else
	{
		m_parent->AppendChild(this);
	}

	std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
	int nameIndex = 0;
	std::string checkName(m_name);
	while(s_clockMap[checkName])
	{
		checkName = m_name + std::to_string((long double)nameIndex);
		++nameIndex;
	}
	
	m_name = checkName;
	s_clockMap[m_name] = this;
}


Clock::~Clock(void)
{
	deleteVectorOfPointer(m_childClocks);
}


void Clock::AppendChild(Clock* clock)
{
	m_childClocks.push_back(clock);
}


void Clock::RemoveClock(Clock* clock)
{
	std::map<std::string,Clock*>::iterator it = s_clockMap.begin();
	while(it != s_clockMap.end())
	{
		Clock* currentClock = it->second;
		if(currentClock == clock)
		{
			s_clockMap.erase(it);
			break;
		}
		++it;
	}

	delete clock;
	clock = nullptr;
}


void Clock::RemoveChild(Clock* clock)
{
	std::vector<Clock*>::iterator it = m_childClocks.begin();
	while(it != m_childClocks.end())
	{
		Clock* currentClock = *it;
		if(currentClock == clock || !clock)
		{
			m_childClocks.erase(it);
			RemoveClock(currentClock);
			break;
		}
		++it;
	}
}


void Clock::Dispose(bool appendChildToParent /* = false */)
{
	if(appendChildToParent)
	{
		if(m_parent != nullptr)
		{
			std::vector<Clock*>::iterator it = m_childClocks.begin();
			for( ; it != m_childClocks.end(); ++it)
			{
				Clock* currentClock = *it;
				m_parent->AppendChild(currentClock);
			}

			m_parent->RemoveChild(this);
		}
		else
		{
			//assert
		}
	}
	else
	{
		if(m_parent)
			m_parent->RemoveChild(this);
		else if(this == s_rootClock)
			s_rootClock = nullptr;
		else
		{
			//assert
		}

		delete this;
	}
}


double Clock::GetAbsoluteTimeSeconds()
{
	static LARGE_INTEGER initialTime;
	static double secondsPerCount = IntializeTimeSystem( initialTime );
	LARGE_INTEGER currentCount;
	QueryPerformanceCounter( &currentCount );
	LONGLONG elapsedCountsSinceInitialTime = currentCount.QuadPart - initialTime.QuadPart;

	double currentSeconds = static_cast< double >( elapsedCountsSinceInitialTime ) * secondsPerCount;
	return currentSeconds;
}


void Clock::AdvanceTime(double deltaTime)
{
	if(m_pause)
		return;

	if(deltaTime > m_maxDeltaTimeStep)
		deltaTime = m_maxDeltaTimeStep;

	std::vector<Clock*>::iterator clock_it = m_childClocks.begin();
	for( ; clock_it != m_childClocks.end(); ++clock_it)
	{
		Clock* currentClock = *clock_it;
		currentClock->AdvanceTime(deltaTime);
	}

	std::vector<Alarm*>::iterator alarm_it = m_alarms.begin();
	while(alarm_it != m_alarms.end())
	{
		Alarm* currentAlarm = *alarm_it;
		currentAlarm->AdvanceTime(deltaTime);
		if(currentAlarm->m_destroy)
		{
			alarm_it = m_alarms.erase(alarm_it);
			delete currentAlarm;
			currentAlarm = nullptr;
		}
		else
			++alarm_it;
	}

	m_currentTimeSeconds += deltaTime * m_timeScale;
	m_lastTimeStep = deltaTime;
}


double Clock::GetTime()
{
	return m_currentTimeSeconds;
}


void Clock::Pause()
{
	m_pause = true;
}


void Clock::UnPause()
{
	m_pause = false;
}


void Clock::AppendAlarm(Alarm* alarm)
{
	alarm->m_attachedClock = this;
	m_alarms.push_back(alarm);
}


void Clock::RemoveAlarm(const Alarm* alarm)
{
	std::vector<Alarm*>::iterator it = m_alarms.begin();
	while(it != m_alarms.end())
	{
		Alarm* currentAlarm = *it;
		if( alarm == currentAlarm )
		{
			m_alarms.erase(it);
			break;
		}
		++it;
	}
}


Alarm* Clock::FindAlarm(const std::string& name)
{
	std::vector<Alarm*>::iterator it = m_alarms.begin();
	while(it != m_alarms.end())
	{
		Alarm* currentAlarm = *it;
		if(currentAlarm->m_name == name)
			return currentAlarm;

		++it;
	}

	return nullptr;
}


void Alarm::FireCallbackFunction()
{
	std::string rawArgs = m_args.m_rawArgString;
	bool sectionStarted = true;
	bool insideSection = false;
	std::string temp;
	for(size_t index = 0; index < m_args.m_rawArgString.length(); index++)
	{
		if(rawArgs[index] == '\"' || (!insideSection && rawArgs[index] == ' '))
		{
			sectionStarted = !sectionStarted;
			if(rawArgs[index] == '\"')
				insideSection = !insideSection;
		}

		if(sectionStarted)
			temp += rawArgs[index];
		if(!sectionStarted || index == rawArgs.length()-1)
		{
			if(temp.length() != 0)
			{
				m_args.m_argList.push_back(temp);
				temp.clear();
			}

			sectionStarted = !sectionStarted;
		}
	}

	m_callbackFunction( m_args );
}


void Alarm::AdvanceTime(double deltaSeconds)
{
	m_currentTime += deltaSeconds;
	if(m_currentTime >= m_period)
	{
		FireCallbackFunction();
		if(m_repeat)
			m_currentTime = 0.0;
		else
			m_destroy = true;
	}
}


};