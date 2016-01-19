#pragma once

#ifndef COMMANDLETREGISTRATION_HPP 
#define COMMANDLETREGISTRATION_HPP

#include "Engine\Commandlet\Commandlet.hpp"
#include <map>

namespace Henry
{

class CommandletRegistration;
typedef std::map<std::string,CommandletRegistration*> CommandletRegistrationMap;
typedef Commandlet* (CommandCreateFunc)(const CommandletArguments* args);

class CommandletRegistration
{
public:
	CommandletRegistration(const std::string name , CommandCreateFunc* creationFunc);
	~CommandletRegistration(void);
	Commandlet* CreateThisCommand(const CommandletArguments* args);
	static CommandletRegistrationMap* GetCommandRegistrationMap();

public:
	static CommandletRegistrationMap* s_commandRegistrationMap;

protected:
	CommandCreateFunc* m_registrationFunc;

private:
	std::string m_name;
};


};

#endif