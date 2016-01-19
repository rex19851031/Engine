#include "CommandletRegistration.hpp"

#include <algorithm>


namespace Henry
{

CommandletRegistrationMap* CommandletRegistration::s_commandRegistrationMap;


CommandletRegistration::CommandletRegistration(const std::string name , CommandCreateFunc* creationFunc) : m_name(name) , m_registrationFunc(creationFunc)
{
	std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);

	if(!s_commandRegistrationMap)
		s_commandRegistrationMap = new CommandletRegistrationMap();

	(*s_commandRegistrationMap)[m_name] = this;
}


CommandletRegistration::~CommandletRegistration(void)
{
}


CommandletRegistrationMap* CommandletRegistration::GetCommandRegistrationMap()
{
	return s_commandRegistrationMap;
}


Commandlet* CommandletRegistration::CreateThisCommand(const CommandletArguments* args)
{
	return (*m_registrationFunc)(args); 
}


};