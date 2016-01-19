#include "Engine\Core\DeveloperConsole.hpp"
#include "Engine\Renderer\OpenGLRenderer.hpp"


namespace Henry
{

extern DeveloperConsole* _console = nullptr;

static void Command_Help(const CommandConsoleArgs& args)
{
	if(args.m_argList.size() == 1)
	{
		_console->ShowAllRegisteredCmd();
	}
	else if(args.m_argList.size() == 2)
	{
		_console->ShowCommandDesc(args.m_argList[1].c_str());
	}
	else
	{
		_console->DrawSentence("ERROR: Wrong Arguments. Usage : <Help> <Command>.",RGBA(1.0f,0.0f,0.0f,1.0f));
	}
};


static void Command_Clear()
{
	_console->ClearScreen();
};


static void Command_Quit()
{
	//_isQuitting = true;
};


DeveloperConsole::DeveloperConsole(void)
{
	_console = this;

	m_screenSizes = Vec2f( (float)OpenGLRenderer::WINDOW_SIZE.x , (float)OpenGLRenderer::WINDOW_SIZE.y );//Vec2f(1600.0f,900.0f);
	m_backgroundLayer[0] = Vertex_PosColor(Vec3f( 0.0f				,	m_screenSizes.y * 0.05f	,	0.0f) , RGBA(0.4f,0.4f,0.4f,0.6f));
	m_backgroundLayer[1] = Vertex_PosColor(Vec3f( m_screenSizes.x	,	m_screenSizes.y * 0.05f	,	0.0f) , RGBA(0.4f,0.4f,0.4f,0.6f));
	m_backgroundLayer[2] = Vertex_PosColor(Vec3f( m_screenSizes.x	,	m_screenSizes.y			,	0.0f) , RGBA(0.4f,0.4f,0.4f,0.6f));
	m_backgroundLayer[3] = Vertex_PosColor(Vec3f( 0.0f				,	m_screenSizes.y			,	0.0f) , RGBA(0.4f,0.4f,0.4f,0.6f));

	m_backgroundLayer[4] = Vertex_PosColor(Vec3f( 0.0f				,	0.0f					,	0.0f) , RGBA(0.6f,0.6f,0.6f,0.5f));
	m_backgroundLayer[5] = Vertex_PosColor(Vec3f( m_screenSizes.x	,	0.0f					,	0.0f) , RGBA(0.6f,0.6f,0.6f,0.5f));
	m_backgroundLayer[6] = Vertex_PosColor(Vec3f( m_screenSizes.x	,	m_screenSizes.y * 0.05f	,	0.0f) , RGBA(0.6f,0.6f,0.6f,0.5f));
	m_backgroundLayer[7] = Vertex_PosColor(Vec3f( 0.0f				,	m_screenSizes.y * 0.05f	,	0.0f) , RGBA(0.6f,0.6f,0.6f,0.5f));

	bool success;
	BitmapFont* font = new BitmapFont("./Data/BMPFonts/arial_normal.fnt" , &success);
	m_registeredFont["arial"] = font;
	font = new BitmapFont("./Data/BMPFonts/buxton.fnt" , &success);
	m_registeredFont["buxton"] = font;
	font = new BitmapFont("./Data/BMPFonts/bitFont.fnt" , &success);
	m_registeredFont["bitFont"] = font;
	m_defaultFont = font;
	m_defaultFontSize = 45.0f;
	
	m_logLines.push_back(CommandLogLine("",RGBA(),m_defaultFont,m_defaultFontSize));
	DrawSentence("Done Initialize!",RGBA(1.0f,1.0f,1.0f,1.0f),45.0f,m_registeredFont["buxton"]);
	m_currentCursorIndex = 0;
	m_currentStartDrawingLineNum = 0;
	m_cursorBlinkSeconds = 0.5f;
	m_cursorTimeCounter = m_cursorBlinkSeconds;
	m_drawCursor = true;
	m_showErrorMsg = true;

	RegisteredCommand* help = new RegisteredCommand("help","Help  => Get instructions about the commands. Usage : <Help> <Help>",Command_Help);
	m_registeredCmds["help"] = help;

	RegisteredCommand* quit = new RegisteredCommand("quit","Quit  => Exit the program.",Command_Quit);
	m_registeredCmds["quit"] = quit;
	
	RegisteredCommand* clear = new RegisteredCommand("clear","Clear => Clear the console log.",Command_Clear);
	m_registeredCmds["clear"] = clear;
}


DeveloperConsole::~DeveloperConsole(void)
{

}


void DeveloperConsole::Draw()
{
	m_widthTable.clear();
	OpenGLRenderer::BindWhiteTexture();
	OpenGLRenderer::DrawVertexWithVertexArray2D(m_backgroundLayer,8,OpenGLRenderer::SHAPE_QUADS,m_screenSizes.x,m_screenSizes.y);
	
	for(size_t index = 1; index < m_logLines.size(); index++)
	{
		size_t LogStartLine = m_logLines.size() - index - m_currentStartDrawingLineNum;
		if(LogStartLine != 0)
			DrawSentence(m_logLines[index].text.c_str(),m_logLines.size() - index - m_currentStartDrawingLineNum,m_logLines[index].fontSize,m_logLines[index].color,m_logLines[index].font);
	}
	
	DrawSentence(m_logLines[0].text.c_str(),0,m_logLines[0].fontSize,RGBA(),m_logLines[0].font);
	DrawCursor();
}


void DeveloperConsole::Update(float deltaTime)
{
	m_cursorTimeCounter -= deltaTime;

	if(m_cursorTimeCounter < 0.0f)
	{
		m_cursorTimeCounter = m_cursorBlinkSeconds;
		m_drawCursor = !m_drawCursor;
	}
}


void DeveloperConsole::DrawCursor()
{
	if(!m_drawCursor)
		return;

	OpenGLRenderer::LineWidth(3.0f);
	Vertex_PosColor cursorPosition[2];

	if(m_currentCursorIndex >= 1)
	{
		cursorPosition[0] = Vertex_PosColor(Vec3f( m_widthTable[m_currentCursorIndex-1] , 1.0f	, 0.0f),RGBA(1.0f,0.0f,0.0f,1.0f));
		cursorPosition[1] = Vertex_PosColor(Vec3f( m_widthTable[m_currentCursorIndex-1] , 44.0f	, 0.0f),RGBA(1.0f,0.0f,0.0f,1.0f));
	}
	else
	{
		cursorPosition[0] = Vertex_PosColor(Vec3f( 1.0f , 1.0f	, 0.0f),RGBA(1.0f,0.0f,0.0f,1.0f));
		cursorPosition[1] = Vertex_PosColor(Vec3f( 1.0f , 44.0f	, 0.0f),RGBA(1.0f,0.0f,0.0f,1.0f));
	}

	OpenGLRenderer::BindWhiteTexture();
	OpenGLRenderer::DrawVertexWithVertexArray2D(cursorPosition,2,OpenGLRenderer::SHAPE_LINES,m_screenSizes.x,m_screenSizes.y);
}


void DeveloperConsole::DrawSentence(const char* sentence , int lineNum , float fontHeight , RGBA color , BitmapFont* font)
{
	m_widthTable.clear();

	if(font == nullptr)
		font = m_defaultFont;

	std::vector<Vertex_PCT> vertices;
	size_t length = strlen(sentence);
	vertices.reserve(length * 4);
	float width = 0;
	std::map<int,GlyphMetaData>::iterator it;
	for(size_t index = 0; index < length; index++)
	{
		it = font->m_glyphData.find(sentence[index]);
		GlyphMetaData gmd = it->second;
		width += fontHeight * gmd.m_ttfA;
		Vec2f min_pos = Vec2f(width , lineNum * fontHeight );
		width += fontHeight * gmd.m_ttfB;
		Vec2f max_pos = Vec2f(width , (lineNum + 1) * fontHeight );
		width += fontHeight * gmd.m_ttfC;

		Vertex_PCT vertex[4];
		vertex[0].texCoords = gmd.m_textCoords.min;
		vertex[0].position = Vec3f( min_pos.x , max_pos.y , 0.0f);
		vertex[0].color = color;

		vertex[1].texCoords = Vec2f(gmd.m_textCoords.min.x , gmd.m_textCoords.max.y);
		vertex[1].position = Vec3f( min_pos.x , min_pos.y , 0.0f);
		vertex[1].color = color;

		vertex[2].texCoords = gmd.m_textCoords.max;
		vertex[2].position = Vec3f( max_pos.x , min_pos.y , 0.0f);
		vertex[2].color = color;

		vertex[3].texCoords = Vec2f(gmd.m_textCoords.max.x,gmd.m_textCoords.min.y);
		vertex[3].position = Vec3f( max_pos.x , max_pos.y , 0.0f);
		vertex[3].color = color;
		
		vertices.push_back(vertex[0]);
		vertices.push_back(vertex[1]);
		vertices.push_back(vertex[2]);
		vertices.push_back(vertex[3]);

		m_widthTable.push_back(width);
	}

	OpenGLRenderer::BindTexture(font->m_glyphSheet->m_textureID);
	OpenGLRenderer::DrawVertexWithVertexArray2D(vertices,OpenGLRenderer::SHAPE_QUADS,m_screenSizes.x,m_screenSizes.y);
}


bool DeveloperConsole::PassCharToLog(char word)
{
	if(word == '`')
		return false;

	std::map<int,GlyphMetaData>::iterator it;
	it =  m_defaultFont->m_glyphData.find( word );
	if(it == m_defaultFont->m_glyphData.end())
		return true;

	m_logLines[0].text.insert(m_currentCursorIndex,1,word);
	m_currentCursorIndex++;
	return true;
}


void DeveloperConsole::SelectByMousePosition(Vec2f mousePos)
{
	if((m_screenSizes.y - mousePos.y) > m_screenSizes.y * 0.05f)
		return;

	if(m_logLines[0].text.length() == 0)
		return;

	if(mousePos.x < m_widthTable[0])
	{
		m_currentCursorIndex = 0;
		return;
	}

	if(mousePos.x > m_widthTable[m_widthTable.size()-1])
	{
		m_currentCursorIndex = m_widthTable.size();
		return;
	}

	for(size_t index = 0; index < m_widthTable.size()-1; index++)
	{
		if((mousePos.x) >= m_widthTable[index] && (mousePos.x) <= m_widthTable[index+1])
		{
			m_currentCursorIndex = index+1;
			m_cursorTimeCounter = m_cursorBlinkSeconds;
			m_drawCursor = true;
			return;
		}
	}
}


bool DeveloperConsole::GetSpecialInput(unsigned char key)
{
	unsigned char BACKSPACE = 8;
	unsigned char UP_ARROW = 38;
	unsigned char DOWN_ARROW = 40;
	unsigned char LEFT_ARROW = 37;
	unsigned char RIGHT_ARROW = 39;
	unsigned char ENTER = 13;
	unsigned char HOME = 36;
	unsigned char END = 35;
	unsigned char DEL = 46;
	unsigned char ESCAPE = 27;
	unsigned char TILT = 96;

	m_drawCursor = true;
	m_cursorTimeCounter = m_cursorBlinkSeconds;

	if(key == TILT)
	{
		return false;
	}

	if(key == HOME)
	{
		m_currentCursorIndex = 0;
		return true;
	}

	if(key == END)
	{
		m_currentCursorIndex = m_logLines[0].text.length();
		return true;
	}

	if(key == BACKSPACE && m_currentCursorIndex != 0)
	{
		m_logLines[0].text.erase(m_currentCursorIndex-1,1);
		m_currentCursorIndex--;
		return true;
	}

	if(key == DEL && m_currentCursorIndex != (int)m_logLines[0].text.length())
	{
		m_logLines[0].text.erase(m_currentCursorIndex,1);
		return true;
	}

	if(key == UP_ARROW)
	{
		if(m_logLines.size() - m_currentStartDrawingLineNum > 20)
			m_currentStartDrawingLineNum++;
	}

	if(key == DOWN_ARROW)
	{
		m_currentStartDrawingLineNum--;
		if(m_currentStartDrawingLineNum < 0)
			m_currentStartDrawingLineNum = 0;
	}

	if(key == LEFT_ARROW)
	{
		if(m_currentCursorIndex != 0)
		{
			m_currentCursorIndex--;
		}
		return true;
	}

	if(key == RIGHT_ARROW)
	{
		if(m_currentCursorIndex != (int)m_logLines[0].text.length())
		{
			m_currentCursorIndex++;
		}
		return true;
	}

	if(key == ENTER)
	{
		if(m_logLines[0].text.length() == 0)
			return false;

		ExecuteCommand(m_logLines[0].text.c_str());
		return true;
	}

	if(key == ESCAPE)
	{
		if(m_logLines[0].text.length() != 0)
		{
			ClearInputField();
			return true;
		}
		else
			return false;
	}

	return true;
}


void DeveloperConsole::ExecuteCommand(const std::string& text)
{
	std::string lowerCaseCmd;
	for(size_t index = 0; index < text.length(); index++)
	{
		lowerCaseCmd += ToLower(text[index]);
	}

	CommandConsoleArgs args;
	args.m_rawArgString = text;
	m_prevInput = text;

	bool sectionStarted = true;
	bool insideSection = false;
	std::string temp;
	for(size_t index = 0; index < lowerCaseCmd.length(); index++)
	{
		if(lowerCaseCmd[index] == '\"' || (!insideSection && lowerCaseCmd[index] == ' '))
		{
			sectionStarted = !sectionStarted;
			if(lowerCaseCmd[index] == '\"')
				insideSection = !insideSection;
		}

		if(sectionStarted)
			temp += lowerCaseCmd[index];
		if(!sectionStarted || index == text.length()-1)
		{
			if(temp.length() != 0)
			{
				args.m_argList.push_back(temp);
				temp.clear();
			}

			sectionStarted = !sectionStarted;
// 			if(temp.length() != 0)
// 			{
// 				CommandLogLine error = CommandLogLine(temp,RGBA());
// 				m_logLines.push_back(error);
// 				temp.clear();
// 			}
// 			sectionStarted = !sectionStarted;
		}
	}

// 	m_logLines[0].text.clear();
// 	m_widthTable.clear();
// 	m_currentCursorIndex = 0;

	if(args.m_argList.empty())
	{
		ClearInputField();
		return;
	}

	std::map<std::string,RegisteredCommand*>::iterator it = m_registeredCmds.find(args.m_argList[0]);
	if(it == m_registeredCmds.end())
	{
		std::string msg = text;
		if(m_showErrorMsg)
		{
			if(text.length() > 14)
				msg = text.substr(0,12) + "..";

			CommandLogLine error = CommandLogLine("ERROR: Command \"" + msg + "\" is not valid. Type \"Help\" for more instructions.",RGBA(1.0f,0.0f,0.0f,1.0f),m_defaultFont,m_defaultFontSize);
			m_logLines.push_back(error);
		}
		else
		{
			CommandLogLine log = CommandLogLine(msg,RGBA(1.0f,0.0f,0.0f,1.0f),m_defaultFont,m_defaultFontSize);
			m_logLines.push_back(log);
		}
	}
	else
	{
		RegisteredCommand rc = *it->second;
		if(rc.m_isFuncWithArgu)
			rc.m_funcWithArgs(args);
		else
			rc.m_func();
	}

	ClearInputField();
}


void DeveloperConsole::ClearScreen()
{
	m_logLines.clear();
	m_currentCursorIndex = 0;
	m_logLines.push_back(CommandLogLine("",RGBA(),m_defaultFont,m_defaultFontSize));
}


void DeveloperConsole::DrawSentence(const char* sentence , RGBA color , float fontSize , BitmapFont* font)
{
	CommandLogLine line(sentence , color , font ? font : m_defaultFont,fontSize);
	m_logLines.push_back(line);
	ClearInputField();
	m_currentCursorIndex = 0;
}


void DeveloperConsole::RegisterCommand(const std::string& name,std::string desc,CmdFuncWithArgs func)
{
	RegisteredCommand* newCmd = new RegisteredCommand(name,desc,func);
	
	std::string lowerCaseCmd;
	for (size_t index = 0; index < name.length(); index++)
	{
		lowerCaseCmd += ToLower(name[index]);
	}

	m_registeredCmds[lowerCaseCmd] = newCmd;
}


void DeveloperConsole::RegisterCommand(const std::string& name,std::string desc,CmdFunc func)
{
	RegisteredCommand* newCmd = new RegisteredCommand(name,desc,func);

	std::string lowerCaseCmd;
	for (size_t index = 0; index < name.length(); index++)
	{
		lowerCaseCmd += ToLower(name[index]);
	}

	m_registeredCmds[name] = newCmd;
}


void DeveloperConsole::ShowCommandDesc(const char* command)
{
	std::map<std::string,RegisteredCommand*>::iterator it = m_registeredCmds.find(command);
	if(it != m_registeredCmds.end())
		DrawSentence(m_registeredCmds[std::string(command)]->m_desc.c_str(),RGBA(0.0f,0.8f,0.8f,1.0f));
	else
	{
		std::string error = "ERROR : Can't find information about \"" + std::string(command) + "\" command";
		DrawSentence(error.c_str(),RGBA(1.0f,0.0f,0.0f,1.0f));
	}
}


void DeveloperConsole::ShowAllRegisteredCmd()
{
	std::map<std::string,RegisteredCommand*>::iterator it = m_registeredCmds.begin();
	while(it!=m_registeredCmds.end())
	{
		RegisteredCommand rc = *it->second;
		DrawSentence(rc.m_desc.c_str(),RGBA(0.0f,0.8f,0.8f,1.0f));
		it++;
	}
}


char DeveloperConsole::ToLower(char in)
{
	if(in >= 'A' && in <= 'Z')
		return in - ('A'-'a');
	return in;
} 


void DeveloperConsole::ClearInputField()
{
	m_logLines[0].text.clear();
	m_logLines[0].fontSize = m_defaultFontSize;
	m_logLines[0].font = nullptr;
	m_widthTable.clear();
	m_currentCursorIndex = 0;
}

};