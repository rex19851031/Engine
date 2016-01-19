#include "Commandlet.hpp"
#include "Engine\Commandlet\CommandletRegistration.hpp"

#include <algorithm>


namespace Henry
{


Commandlet::Commandlet(const CommandletArguments* args) 
	: m_commandletArgs( args )
	, m_exitAfterExecuted( true )
{

}


Commandlet::~Commandlet(void)
{

}


Commandlet* Commandlet::CreateAndGetCommand(const std::string& name , const CommandletArguments* args)
{
	if (!CommandletRegistration::GetCommandRegistrationMap())
		return nullptr;

	std::string targetName = name;
	std::transform(targetName.begin(), targetName.end(), targetName.begin(), ::tolower);

	CommandletRegistrationMap* registrationCommandMap = CommandletRegistration::GetCommandRegistrationMap();

	CommandletRegistrationMap::iterator iter = registrationCommandMap->begin();
	for( ; iter != registrationCommandMap->end(); ++iter)	//Find() in map class , return iterator
	{
		CommandletRegistration* registration = iter->second;
		if(iter->first == targetName)
		{
			Commandlet* thisBehavior = registration->CreateThisCommand( args );
			return thisBehavior;
		}
	}
	
	return nullptr;
}


bool Commandlet::AnalysisAndRunCommandlet(const std::string& commandLineString)
{
	bool inQuote = false;
	bool inArgument = false;
	bool inCommand = false;

	std::vector<CommandletArguments> commandList;
	CommandletArguments _commandletArguments;
	std::string command;
	std::string argument;

	for( size_t index = 0; index < commandLineString.size(); ++index )
	{
		char current = commandLineString[index];
		_commandletArguments.rawString += current;

		if( ( inCommand || current == '-' ) && !inQuote && !inArgument )
		{
			if( current == '-' && _commandletArguments.command.size() != 0 )
			{
				commandList.push_back( _commandletArguments );
				
				std::vector<std::string> empty;
				_commandletArguments.arguments.clear();
				_commandletArguments.arguments = empty;
				_commandletArguments.command.clear();
				_commandletArguments.rawString.clear();
			}

			if( inCommand && current != ' ' )
				command += current;

			inCommand = true;
		}

		if( current == '"' )
		{
			inQuote = !inQuote;
			inArgument = inQuote;
			inCommand = inQuote;
		}

		if( current == ' ' && !inQuote )
		{
			inArgument = false;
			inCommand = false;
		}

		if( current != ' ' && current != '"' && current != '-' && !inCommand )
		{
			inArgument = true;
			inCommand = false;
		}

		if( inArgument && current != '"' )
		{
			argument += current;
		}

		if( !inArgument && argument.size() != 0 )
		{
			_commandletArguments.arguments.push_back(argument);
			argument.clear();
		}

		if( !inCommand && command.size() != 0 )
		{
			_commandletArguments.command = command;
			command.clear();
		}

		if( index + 1 == commandLineString.size() )
		{
			if( argument.size() != 0 )
				_commandletArguments.arguments.push_back( argument );

			commandList.push_back(_commandletArguments);
		}
	}

	bool exitAfterExecuted = false;
	for(size_t index = 0; index < commandList.size(); ++index )
	{
		Commandlet* theCommand = CreateAndGetCommand( commandList[index].command , &commandList[index] );
		if(theCommand)
		{
			bool _exit = theCommand->Execute();
			if(_exit)
				exitAfterExecuted = true;
		}
	}

	return exitAfterExecuted;
}


bool Commandlet::Execute()
{
	return m_exitAfterExecuted;
}


};