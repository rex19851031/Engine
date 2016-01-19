#pragma once

#ifndef PROFILING_HPP
#define PROFILING_HPP

#include <stack>
#include <map>
#include <string>

#include "Engine\Core\Clock.hpp"
#include "Engine\Renderer\BitmapFont.hpp"


namespace Henry
{

class Profiling
{
public:
	Profiling(const std::string& name);
	~Profiling(void);
	void AppendProfiling(Profiling* _profiling);
	static void Render(BitmapFont* font);
	static void Start(const std::string& name);
	static void Stop();
	static void RemoveAllProfiling();
	double GetElapsedTime();
	double GetAverageTime();
	double GetPercentage();

public:
	static std::stack<Profiling*> s_profilingStack;
	static std::map<std::string,Profiling*> s_profilingMap;
	std::vector<Profiling*> m_subProfiling;
	std::string m_name;
	double m_startTime;
	double m_stopTime;
	double m_lifeTime;
	int m_layer;
	int m_frameCount;
	static bool s_display;
	Profiling* m_parent;
};

};

#endif