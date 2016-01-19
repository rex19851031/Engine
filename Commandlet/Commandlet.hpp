#pragma once

#ifndef COMMANDLETS_HPP
#define COMMANDLETS_HPP

#include <string>
#include <vector>

namespace Henry
{

struct CommandletArguments
{
	std::string rawString;
	std::string command;
	std::vector<std::string> arguments;
};


class Commandlet
{
public:
	Commandlet(const CommandletArguments* args);
	~Commandlet(void);
	virtual bool Execute();
	static Commandlet* CreateAndGetCommand(const std::string& name , const CommandletArguments* args);
	static bool AnalysisAndRunCommandlet(const std::string& commandLineString);
public:
	const CommandletArguments* m_commandletArgs;
	bool m_exitAfterExecuted;
};

};

#endif