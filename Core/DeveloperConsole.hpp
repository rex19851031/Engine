#pragma once

#ifndef DEVELOPERCONSOLE_HPP 
#define DEVELOPERCONSOLE_HPP

#include <map>
#include <vector>
#include <string>

#include "VertexStruct.hpp"
#include "Engine\Renderer\BitmapFont.hpp"

namespace Henry
{

struct CommandConsoleArgs
{
	std::string m_rawArgString;
	std::vector<std::string> m_argList;
};

typedef void(CmdFunc)();
typedef void(CmdFuncWithArgs)(const CommandConsoleArgs& args);

struct RegisteredCommand
{
	RegisteredCommand(){};
	RegisteredCommand(std::string name,std::string desc,CmdFunc func) : m_name(name),m_desc(desc),m_func(func),m_isFuncWithArgu(false) {};
	RegisteredCommand(std::string name,std::string desc,CmdFuncWithArgs func) : m_name(name),m_desc(desc),m_funcWithArgs(func),m_isFuncWithArgu(true) { }

	CmdFunc* m_func;
	CmdFuncWithArgs* m_funcWithArgs;
	std::string m_desc;
	std::string m_name;
	bool m_isFuncWithArgu;
};


struct CommandLogLine
{
	CommandLogLine(BitmapFont* bmpFont , float size) : font(bmpFont) , fontSize(size) {};
	CommandLogLine(std::string rawCommand,RGBA commandColor,BitmapFont* bmpFont,float size) : text(rawCommand) , color(commandColor) , font(bmpFont) , fontSize(size) {};
	std::string text;
	RGBA color;
	BitmapFont* font;
	float fontSize;
};


class DeveloperConsole
{
public:
	DeveloperConsole(void);
	~DeveloperConsole(void);
	void Draw();
	void Update(float deltaTime);
	bool PassCharToLog(char word);
	void DrawCursor();
	bool GetSpecialInput(unsigned char key);
	void SelectByMousePosition(Vec2f mousePos);
	void ExecuteCommand(const std::string& text);
	void RegisterCommand(const std::string& name,std::string desc,CmdFunc func);
	void RegisterCommand(const std::string& name,std::string desc,CmdFuncWithArgs func);
	void ClearScreen();
	void DrawSentence(const char* sentence , RGBA color , float fontSize = 45.0f, BitmapFont* font = nullptr);
	void ShowCommandDesc(const char* command);
	void ShowAllRegisteredCmd();

public:
	std::string m_prevInput;
	bool m_showErrorMsg;

private:
	void DrawSentence(const char* sentence , int lineNum , float fontHeight , RGBA color , BitmapFont* font);
	void ClearInputField();
	char ToLower(char in);
	std::map<std::string,RegisteredCommand*> m_registeredCmds;
	std::map<std::string,BitmapFont*> m_registeredFont;
	std::vector<CommandLogLine> m_logLines;
	std::vector<float> m_widthTable;
	Vertex_PosColor m_backgroundLayer[8];
	size_t m_currentCursorIndex;
	int m_currentStartDrawingLineNum;
	float m_cursorTimeCounter;
	float m_cursorBlinkSeconds;
	bool m_drawCursor; 
	Vec2f m_screenSizes;
	BitmapFont* m_defaultFont;
	float m_defaultFontSize;
};

extern DeveloperConsole* _console;

#endif

};