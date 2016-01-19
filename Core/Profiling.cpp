#include "Profiling.hpp"

#include "Engine\Renderer\OpenGLRenderer.hpp"
#include "Engine\Core\HenryFunctions.hpp"
#include "Engine\Core\Time.hpp"


namespace Henry
{

std::stack<Profiling*> Profiling::s_profilingStack;
std::map<std::string,Profiling*> Profiling::s_profilingMap;
bool Profiling::s_display = true;

Profiling::Profiling(const std::string& name) 
	: m_name(name)
	, m_layer(0)
	, m_stopTime(-1.0)
	, m_frameCount(0)
	, m_lifeTime(0.0)
	, m_parent(nullptr)
{
	m_startTime = GetCurrentTimeSeconds();
}


Profiling::~Profiling(void)
{
}


void Profiling::Start(const std::string& name)
{
	Profiling* _profiling;

	if(s_profilingMap[name])
	{
		_profiling = s_profilingMap[name];
		_profiling->m_startTime = GetCurrentTimeSeconds();
		_profiling->m_stopTime = -1.0;
	}
	else
	{
		_profiling = new Profiling(name);

		if(s_profilingStack.size() != 0)
		{
			s_profilingStack.top()->AppendProfiling(_profiling);
		}

		s_profilingMap[name] = _profiling;
	}

	++_profiling->m_frameCount;
	s_profilingStack.push(_profiling);
}


void Profiling::Stop()
{
	Profiling* _profiling = s_profilingStack.top();
	s_profilingStack.pop();
	_profiling->m_stopTime = GetCurrentTimeSeconds();
	_profiling->m_lifeTime += _profiling->GetElapsedTime();
}


double Profiling::GetElapsedTime()
{
	if(m_stopTime != -1.0)
		return m_stopTime - m_startTime;
	return GetCurrentTimeSeconds() - m_startTime;
}


void Profiling::AppendProfiling(Profiling* _profiling)
{
	_profiling->m_parent = this;
	_profiling->m_layer = m_layer + 1;
	m_subProfiling.push_back(_profiling);
}


void Profiling::RemoveAllProfiling()
{
	std::map<std::string,Profiling*>::iterator it = s_profilingMap.begin();
	while( it != s_profilingMap.end() )
	{
		Profiling* currentProfiling = it->second;
		it = s_profilingMap.erase(it);
		delete currentProfiling;
		currentProfiling = nullptr;
	}
}


double Profiling::GetAverageTime()
{
	return m_lifeTime / m_frameCount;
}


double Profiling::GetPercentage()
{
	if(m_layer != 0 && m_parent)
	{
		return GetElapsedTime() / m_parent->GetElapsedTime();
	}

	return 1.0;
}


void Profiling::Render(BitmapFont* font)
{
	if(!s_display)
		return;

	Vec2f position( 0.0f , 850.0f );
	std::map<std::string,Profiling*>::iterator it = s_profilingMap.begin();
	while( it != s_profilingMap.end() )
	{
		Profiling* _profiling = it->second;
		position.x = 650.0f + _profiling->m_layer * 50.0f;

		std::string msg;
		msg = _profiling->m_name + " -> %.4lf%% , Total Time : %lf , Average Time : %lf" ;
		font->Draw( msg.c_str() , position , 30 , RGBA() , OpenGLRenderer::WINDOW_SIZE , _profiling->GetPercentage() , _profiling->GetElapsedTime() , _profiling->GetAverageTime() );
		position += Vec2f( 0.0f, -50.0f );

		++it;
	}
}


};