#include "BitmapFont.hpp"

#include <Windows.h>
#include <atlconv.h>
#include <string>

#include "Engine\Renderer\OpenGLRenderer.hpp"
#include "Engine\Parsing\ZipUtils\ZipHelper.hpp"


namespace Henry
{

BitmapFont::BitmapFont(char* fontDocPath , bool* success , bool autoParsing)
{
	m_metaDoc = new	TiXmlDocument(fontDocPath);
	*success = m_metaDoc->LoadFile();
	
	if( !(*success) )
	{
		std::string errorInfo(fontDocPath);
		errorInfo = "Failed to load file : " + errorInfo;
		MessageBoxA( NULL , errorInfo.c_str(), "Failed to loading files", MB_ICONERROR | MB_OK );
	}

	if(m_metaDoc->ErrorId() > 0)
	{
		std::string errorInfo(fontDocPath);
		errorInfo = "Error , can't analysis meta-data file : " + errorInfo + "\n\n";
		MessageBoxA( NULL , (LPCSTR)errorInfo.c_str() , "BMP Fonts Metadata Analysis Failed!", MB_ICONERROR | MB_OK );
	}

	if(autoParsing)
		ParsingXmlDataAndLoadTexture();
}


BitmapFont::BitmapFont(char* fontDocPath , char* zipFilePath , bool* success , bool autoParsing)
{
	int len;
	const unsigned char* xmlBuffer = ZipHelper::GetContentInZip(zipFilePath,fontDocPath,"c23",&len,success);

	m_metaDoc = new TiXmlDocument();
	m_metaDoc->Parse((const char*)xmlBuffer , 0, TIXML_ENCODING_UTF8);
	
	if(m_metaDoc->ErrorId() > 0)
	{
		std::string errorInfo(fontDocPath);
		errorInfo = "Error , can't analysis meta-data file : " + errorInfo + "\n\n";
		MessageBoxA( NULL , (LPCSTR)errorInfo.c_str() , "BMP Fonts Metadata Analysis Failed!", MB_ICONERROR | MB_OK );
	}

	if(autoParsing)
		ParsingXmlDataAndLoadTexture(NONE , zipFilePath);
}


BitmapFont::BitmapFont(BuildInFont font , bool autoParsing)
{
	Initialize();
	m_metaDoc = new TiXmlDocument();
	m_metaDoc->Parse(m_buildInFontXML[font].str().c_str(), 0, TIXML_ENCODING_UTF8);
	if(autoParsing)
		ParsingXmlDataAndLoadTexture(font);
}


BitmapFont::~BitmapFont(void)
{
	delete m_metaDoc;
	delete m_glyphSheet;
}


void BitmapFont::Draw(const std::string& rawSentence, const Vec2f& position, const float& fontHeight, const RGBA& color, const Vec2i canvasCoord, ...)
{
	va_list args;
	va_start( args , canvasCoord );
	std::string sentence = ConvertArgument( rawSentence.c_str(), args);
	va_end(args);

	std::vector<float> empty;
	m_widthTable = empty;
	m_widthTable.clear();
	m_widthTable.reserve(sentence.length());

	std::vector<Vertex_PCT> vertices;
	vertices.reserve(sentence.length() * 4);
	float width = 0;
	std::map<int,GlyphMetaData>::iterator it;
	for(size_t index = 0; index < sentence.length(); ++index)
	{
		it = m_glyphData.find(sentence[index]);
		GlyphMetaData gmd = it->second;
		width += fontHeight * gmd.m_ttfA;
		Vec2f min_pos = position + Vec2f(width , 0.0f );
		width += fontHeight * gmd.m_ttfB;
		Vec2f max_pos = position + Vec2f(width , fontHeight );
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

	OpenGLRenderer::BindTexture(m_glyphSheet->m_textureID);
	OpenGLRenderer::DrawVertexWithVertexArray2D(vertices , OpenGLRenderer::SHAPE_QUADS , (float)canvasCoord.x , (float)canvasCoord.y);
}


std::string BitmapFont::ConvertArgument(const char*message , va_list args)
{
	const int buffer_size = 2048;
	char buffer[buffer_size];
	memset(buffer , 0 , buffer_size);
	_vsnprintf_s(buffer,buffer_size,buffer_size-1,message,args);
	std::string msg(buffer);
	return msg;
}


float BitmapFont::GetWidthOfText(const std::string& rawSentence, float& fontHeight, ...)
{
	va_list args;
	va_start(args, fontHeight);
	std::string sentence = ConvertArgument(rawSentence.c_str(), args);
	va_end(args);

	float width = 0;
	std::map<int, GlyphMetaData>::iterator it;
	for (size_t index = 0; index < sentence.length(); ++index)
	{
		it = m_glyphData.find(sentence[index]);
		GlyphMetaData gmd = it->second;
		width += fontHeight * gmd.m_ttfA;
		width += fontHeight * gmd.m_ttfB;
		width += fontHeight * gmd.m_ttfC;
	}

	return width;
}


void BitmapFont::Initialize()
{
#pragma region Arial_Xml
	m_buildInFontXML[ARIAL]
	<< "	<?xml version=\"1.0\"?>																																							" 
	<< "	<font>																																											"
	<< "	<info face=\"Arial\" size=\"-40\" bold=\"0\" italic=\"0\" charset=\"\" unicode=\"1\" stretchH=\"100\" smooth=\"1\" aa=\"1\" padding=\"0,0,0,0\" spacing=\"1,1\" outline=\"5\"/>	"
	<< "	<common lineHeight=\"45\" base=\"36\" scaleW=\"1024\" scaleH=\"1024\" pages=\"1\" packed=\"0\" alphaChnl=\"1\" redChnl=\"0\" greenChnl=\"0\" blueChnl=\"0\"/>					"
	<< "	<pages>																																											"
	<< "	<page id=\"0\" file=\"arial_0.png\" />																																			"
	<< "	</pages>																																										"
	<< "	<chars count=\"527\">																																							"
	<< "	<char id=\"32\" x=\"684\" y=\"896\" width=\"13\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"11\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"33\" x=\"395\" y=\"896\" width=\"16\" height=\"55\" xoffset=\"-1\" yoffset=\"-5\" xadvance=\"14\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"34\" x=\"670\" y=\"840\" width=\"22\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"14\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"35\" x=\"514\" y=\"392\" width=\"33\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"36\" x=\"496\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"37\" x=\"353\" y=\"56\" width=\"42\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"36\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"38\" x=\"987\" y=\"0\" width=\"36\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"39\" x=\"378\" y=\"896\" width=\"16\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"8\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"40\" x=\"985\" y=\"840\" width=\"20\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"13\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"41\" x=\"84\" y=\"896\" width=\"20\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"13\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"42\" x=\"0\" y=\"840\" width=\"24\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"16\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"43\" x=\"164\" y=\"504\" width=\"31\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"23\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"44\" x=\"495\" y=\"896\" width=\"15\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"11\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"45\" x=\"1000\" y=\"672\" width=\"23\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"13\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"46\" x=\"527\" y=\"896\" width=\"15\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"11\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"47\" x=\"250\" y=\"840\" width=\"23\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"11\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"48\" x=\"527\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"49\" x=\"919\" y=\"840\" width=\"21\" height=\"55\" xoffset=\"-1\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"50\" x=\"589\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"51\" x=\"651\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"52\" x=\"132\" y=\"504\" width=\"31\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"53\" x=\"713\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"54\" x=\"775\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"55\" x=\"868\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"56\" x=\"899\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"57\" x=\"930\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"58\" x=\"607\" y=\"896\" width=\"15\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"11\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"59\" x=\"623\" y=\"896\" width=\"15\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"11\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"60\" x=\"644\" y=\"504\" width=\"31\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"23\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"61\" x=\"612\" y=\"504\" width=\"31\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"23\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"62\" x=\"548\" y=\"504\" width=\"31\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"23\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"63\" x=\"961\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"64\" x=\"654\" y=\"0\" width=\"49\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"41\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"65\" x=\"600\" y=\"112\" width=\"39\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"66\" x=\"355\" y=\"336\" width=\"34\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"67\" x=\"696\" y=\"224\" width=\"37\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"68\" x=\"72\" y=\"336\" width=\"35\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"69\" x=\"854\" y=\"392\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"70\" x=\"516\" y=\"504\" width=\"31\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"71\" x=\"156\" y=\"224\" width=\"38\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"72\" x=\"174\" y=\"392\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"73\" x=\"463\" y=\"896\" width=\"15\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"10\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"74\" x=\"699\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"75\" x=\"482\" y=\"280\" width=\"35\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"76\" x=\"992\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"77\" x=\"720\" y=\"112\" width=\"39\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"33\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"78\" x=\"888\" y=\"392\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"79\" x=\"640\" y=\"112\" width=\"39\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"80\" x=\"956\" y=\"392\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"81\" x=\"320\" y=\"168\" width=\"39\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"82\" x=\"75\" y=\"280\" width=\"36\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"83\" x=\"554\" y=\"280\" width=\"35\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"84\" x=\"105\" y=\"392\" width=\"34\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"85\" x=\"612\" y=\"448\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"86\" x=\"962\" y=\"224\" width=\"37\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"87\" x=\"501\" y=\"0\" width=\"50\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"40\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"88\" x=\"886\" y=\"224\" width=\"37\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"89\" x=\"112\" y=\"280\" width=\"36\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"26\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"90\" x=\"810\" y=\"336\" width=\"34\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"91\" x=\"166\" y=\"896\" width=\"19\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"11\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"92\" x=\"370\" y=\"840\" width=\"23\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"11\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"93\" x=\"146\" y=\"896\" width=\"19\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"11\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"94\" x=\"583\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"19\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"95\" x=\"705\" y=\"336\" width=\"34\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"96\" x=\"284\" y=\"896\" width=\"18\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"13\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"97\" x=\"832\" y=\"504\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"98\" x=\"60\" y=\"728\" width=\"29\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"99\" x=\"380\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"100\" x=\"850\" y=\"672\" width=\"29\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"101\" x=\"0\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"102\" x=\"225\" y=\"840\" width=\"24\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"103\" x=\"730\" y=\"672\" width=\"29\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"104\" x=\"84\" y=\"784\" width=\"27\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"105\" x=\"479\" y=\"896\" width=\"15\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"8\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"106\" x=\"63\" y=\"896\" width=\"20\" height=\"55\" xoffset=\"-8\" yoffset=\"-5\" xadvance=\"10\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"107\" x=\"641\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"108\" x=\"639\" y=\"896\" width=\"15\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"8\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"109\" x=\"942\" y=\"56\" width=\"40\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"34\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"110\" x=\"902\" y=\"728\" width=\"27\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"111\" x=\"62\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"112\" x=\"970\" y=\"672\" width=\"29\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"113\" x=\"940\" y=\"672\" width=\"29\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"114\" x=\"808\" y=\"840\" width=\"22\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"13\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"115\" x=\"467\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"19\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"116\" x=\"875\" y=\"840\" width=\"21\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"11\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"117\" x=\"560\" y=\"784\" width=\"27\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"118\" x=\"484\" y=\"504\" width=\"31\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"19\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"119\" x=\"760\" y=\"168\" width=\"39\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"120\" x=\"119\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"18\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"121\" x=\"430\" y=\"672\" width=\"29\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"19\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"122\" x=\"93\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"123\" x=\"322\" y=\"840\" width=\"23\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"13\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"124\" x=\"559\" y=\"896\" width=\"15\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"11\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"125\" x=\"298\" y=\"840\" width=\"23\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"13\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"126\" x=\"324\" y=\"504\" width=\"31\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"23\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"160\" x=\"670\" y=\"896\" width=\"13\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"11\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"161\" x=\"446\" y=\"896\" width=\"16\" height=\"55\" xoffset=\"-1\" yoffset=\"-5\" xadvance=\"14\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"162\" x=\"124\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"163\" x=\"260\" y=\"504\" width=\"31\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"164\" x=\"155\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"165\" x=\"99\" y=\"504\" width=\"32\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"166\" x=\"575\" y=\"896\" width=\"15\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"11\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"167\" x=\"186\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"168\" x=\"601\" y=\"840\" width=\"22\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"13\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"169\" x=\"818\" y=\"56\" width=\"41\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"170\" x=\"100\" y=\"840\" width=\"24\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"15\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"171\" x=\"612\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"172\" x=\"356\" y=\"504\" width=\"31\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"23\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"173\" x=\"346\" y=\"840\" width=\"23\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"13\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"174\" x=\"566\" y=\"56\" width=\"41\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"175\" x=\"390\" y=\"336\" width=\"34\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"176\" x=\"647\" y=\"840\" width=\"22\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"16\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"177\" x=\"217\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"178\" x=\"1000\" y=\"224\" width=\"23\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"13\" page=\"0\" chnl=\"15\" />										"
	<< "	<char id=\"179\" x=\"1000\" y=\"168\" width=\"23\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"13\" page=\"0\" chnl=\"15\" />										"
	<< "	<char id=\"180\" x=\"303\" y=\"896\" width=\"18\" height=\"55\" xoffset=\"-1\" yoffset=\"-5\" xadvance=\"13\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"181\" x=\"580\" y=\"672\" width=\"29\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"23\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"182\" x=\"990\" y=\"392\" width=\"33\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"183\" x=\"543\" y=\"896\" width=\"15\" height=\"55\" xoffset=\"-1\" yoffset=\"-5\" xadvance=\"13\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"184\" x=\"105\" y=\"896\" width=\"20\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"13\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"185\" x=\"226\" y=\"896\" width=\"19\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"13\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"186\" x=\"861\" y=\"784\" width=\"25\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"15\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"187\" x=\"177\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"188\" x=\"309\" y=\"56\" width=\"43\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"33\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"189\" x=\"439\" y=\"56\" width=\"42\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"33\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"190\" x=\"45\" y=\"56\" width=\"43\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"33\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"191\" x=\"279\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"192\" x=\"120\" y=\"112\" width=\"39\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"193\" x=\"520\" y=\"168\" width=\"39\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"194\" x=\"200\" y=\"112\" width=\"39\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"195\" x=\"560\" y=\"112\" width=\"39\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"196\" x=\"760\" y=\"112\" width=\"39\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"197\" x=\"400\" y=\"168\" width=\"39\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"198\" x=\"603\" y=\"0\" width=\"50\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"40\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"199\" x=\"506\" y=\"224\" width=\"37\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"200\" x=\"0\" y=\"448\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"201\" x=\"68\" y=\"448\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"202\" x=\"170\" y=\"448\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"203\" x=\"238\" y=\"448\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"204\" x=\"360\" y=\"896\" width=\"17\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"11\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"205\" x=\"1006\" y=\"840\" width=\"17\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"11\" page=\"0\" chnl=\"15\" />										"
	<< "	<char id=\"206\" x=\"693\" y=\"840\" width=\"22\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"11\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"207\" x=\"963\" y=\"840\" width=\"21\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"11\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"208\" x=\"640\" y=\"168\" width=\"39\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"209\" x=\"306\" y=\"448\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"210\" x=\"520\" y=\"112\" width=\"39\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"211\" x=\"480\" y=\"112\" width=\"39\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"212\" x=\"440\" y=\"112\" width=\"39\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"213\" x=\"280\" y=\"112\" width=\"39\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"214\" x=\"240\" y=\"112\" width=\"39\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"215\" x=\"554\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"23\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"216\" x=\"160\" y=\"112\" width=\"39\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"217\" x=\"340\" y=\"448\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"218\" x=\"374\" y=\"448\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"219\" x=\"408\" y=\"448\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"220\" x=\"442\" y=\"448\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"221\" x=\"772\" y=\"224\" width=\"37\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"222\" x=\"476\" y=\"448\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"223\" x=\"945\" y=\"448\" width=\"32\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"224\" x=\"341\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"225\" x=\"403\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"226\" x=\"465\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"227\" x=\"527\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"228\" x=\"589\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"229\" x=\"651\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"230\" x=\"0\" y=\"56\" width=\"44\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"36\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"231\" x=\"525\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"232\" x=\"682\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"233\" x=\"713\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"234\" x=\"744\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"235\" x=\"775\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"236\" x=\"322\" y=\"896\" width=\"18\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"11\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"237\" x=\"206\" y=\"896\" width=\"19\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"11\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"238\" x=\"75\" y=\"840\" width=\"24\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"11\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"239\" x=\"486\" y=\"840\" width=\"22\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"11\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"240\" x=\"806\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"241\" x=\"986\" y=\"728\" width=\"27\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"242\" x=\"868\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"243\" x=\"899\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"244\" x=\"930\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"245\" x=\"961\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"246\" x=\"992\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"247\" x=\"0\" y=\"672\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"248\" x=\"520\" y=\"672\" width=\"29\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"23\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"249\" x=\"364\" y=\"784\" width=\"27\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"250\" x=\"28\" y=\"784\" width=\"27\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"251\" x=\"168\" y=\"784\" width=\"27\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"252\" x=\"308\" y=\"784\" width=\"27\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"253\" x=\"820\" y=\"672\" width=\"29\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"254\" x=\"880\" y=\"672\" width=\"29\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"255\" x=\"910\" y=\"672\" width=\"29\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"256\" x=\"0\" y=\"112\" width=\"39\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"257\" x=\"31\" y=\"672\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"258\" x=\"920\" y=\"168\" width=\"39\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"259\" x=\"62\" y=\"672\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"260\" x=\"800\" y=\"168\" width=\"39\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"261\" x=\"420\" y=\"504\" width=\"31\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"262\" x=\"810\" y=\"224\" width=\"37\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"263\" x=\"235\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"264\" x=\"582\" y=\"224\" width=\"37\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"265\" x=\"409\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"266\" x=\"848\" y=\"224\" width=\"37\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"267\" x=\"351\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"268\" x=\"620\" y=\"224\" width=\"37\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"269\" x=\"322\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"270\" x=\"36\" y=\"336\" width=\"35\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"271\" x=\"0\" y=\"336\" width=\"35\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"272\" x=\"680\" y=\"168\" width=\"39\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"273\" x=\"780\" y=\"448\" width=\"32\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"274\" x=\"310\" y=\"392\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"275\" x=\"124\" y=\"672\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"276\" x=\"378\" y=\"392\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"277\" x=\"155\" y=\"672\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"278\" x=\"446\" y=\"392\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"279\" x=\"186\" y=\"672\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"280\" x=\"480\" y=\"392\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"281\" x=\"217\" y=\"672\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"282\" x=\"718\" y=\"392\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"283\" x=\"248\" y=\"672\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"284\" x=\"351\" y=\"224\" width=\"38\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"285\" x=\"30\" y=\"728\" width=\"29\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"286\" x=\"0\" y=\"224\" width=\"38\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"287\" x=\"370\" y=\"672\" width=\"29\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"288\" x=\"78\" y=\"224\" width=\"38\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"289\" x=\"400\" y=\"672\" width=\"29\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"290\" x=\"273\" y=\"224\" width=\"38\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"291\" x=\"610\" y=\"672\" width=\"29\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"292\" x=\"34\" y=\"448\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"293\" x=\"336\" y=\"784\" width=\"27\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"294\" x=\"880\" y=\"168\" width=\"39\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"295\" x=\"708\" y=\"504\" width=\"30\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"296\" x=\"25\" y=\"840\" width=\"24\" height=\"55\" xoffset=\"-7\" yoffset=\"-5\" xadvance=\"10\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"297\" x=\"755\" y=\"784\" width=\"26\" height=\"55\" xoffset=\"-7\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"298\" x=\"555\" y=\"840\" width=\"22\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"10\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"299\" x=\"939\" y=\"784\" width=\"25\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"300\" x=\"762\" y=\"840\" width=\"22\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"10\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"301\" x=\"50\" y=\"840\" width=\"24\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"302\" x=\"341\" y=\"896\" width=\"18\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"10\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"303\" x=\"246\" y=\"896\" width=\"18\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"8\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"304\" x=\"412\" y=\"896\" width=\"16\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"10\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"305\" x=\"511\" y=\"896\" width=\"15\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"306\" x=\"626\" y=\"280\" width=\"35\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"307\" x=\"965\" y=\"784\" width=\"24\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"18\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"308\" x=\"879\" y=\"448\" width=\"32\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"309\" x=\"913\" y=\"784\" width=\"25\" height=\"55\" xoffset=\"-8\" yoffset=\"-5\" xadvance=\"10\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"310\" x=\"914\" y=\"280\" width=\"35\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"311\" x=\"670\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"312\" x=\"873\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"313\" x=\"739\" y=\"504\" width=\"30\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"314\" x=\"265\" y=\"896\" width=\"18\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"8\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"315\" x=\"770\" y=\"504\" width=\"30\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"316\" x=\"126\" y=\"896\" width=\"19\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"8\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"317\" x=\"460\" y=\"672\" width=\"29\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"318\" x=\"42\" y=\"896\" width=\"20\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"319\" x=\"801\" y=\"504\" width=\"30\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"320\" x=\"509\" y=\"840\" width=\"22\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"13\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"321\" x=\"912\" y=\"448\" width=\"32\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"322\" x=\"941\" y=\"840\" width=\"21\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"9\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"323\" x=\"752\" y=\"392\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"324\" x=\"958\" y=\"728\" width=\"27\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"325\" x=\"140\" y=\"392\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"326\" x=\"392\" y=\"784\" width=\"27\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"327\" x=\"344\" y=\"392\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"328\" x=\"0\" y=\"784\" width=\"27\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"329\" x=\"878\" y=\"280\" width=\"35\" height=\"55\" xoffset=\"-7\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"330\" x=\"986\" y=\"280\" width=\"35\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"331\" x=\"844\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"332\" x=\"983\" y=\"56\" width=\"39\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"333\" x=\"863\" y=\"504\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"334\" x=\"440\" y=\"168\" width=\"39\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"335\" x=\"894\" y=\"504\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"336\" x=\"840\" y=\"168\" width=\"39\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"337\" x=\"925\" y=\"504\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"338\" x=\"753\" y=\"0\" width=\"48\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"40\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"339\" x=\"850\" y=\"0\" width=\"46\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"38\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"340\" x=\"371\" y=\"280\" width=\"36\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"341\" x=\"440\" y=\"840\" width=\"22\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"13\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"342\" x=\"408\" y=\"280\" width=\"36\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"343\" x=\"463\" y=\"840\" width=\"22\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"13\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"344\" x=\"38\" y=\"280\" width=\"36\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"345\" x=\"1000\" y=\"112\" width=\"23\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"13\" page=\"0\" chnl=\"15\" />										"
	<< "	<char id=\"346\" x=\"842\" y=\"280\" width=\"35\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"347\" x=\"757\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"19\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"348\" x=\"662\" y=\"280\" width=\"35\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"349\" x=\"815\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"19\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"350\" x=\"518\" y=\"280\" width=\"35\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"351\" x=\"90\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"19\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"352\" x=\"108\" y=\"336\" width=\"35\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"353\" x=\"148\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"354\" x=\"320\" y=\"336\" width=\"34\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"355\" x=\"785\" y=\"840\" width=\"22\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"11\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"356\" x=\"740\" y=\"336\" width=\"34\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"357\" x=\"728\" y=\"784\" width=\"26\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"15\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"358\" x=\"845\" y=\"336\" width=\"34\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"359\" x=\"578\" y=\"840\" width=\"22\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"11\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"360\" x=\"510\" y=\"448\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"361\" x=\"532\" y=\"784\" width=\"27\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"362\" x=\"578\" y=\"448\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"363\" x=\"672\" y=\"784\" width=\"27\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"364\" x=\"272\" y=\"448\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"365\" x=\"616\" y=\"784\" width=\"27\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"366\" x=\"544\" y=\"448\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"367\" x=\"476\" y=\"784\" width=\"27\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"368\" x=\"276\" y=\"392\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"369\" x=\"448\" y=\"784\" width=\"27\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"370\" x=\"412\" y=\"392\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"371\" x=\"790\" y=\"672\" width=\"29\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"372\" x=\"552\" y=\"0\" width=\"50\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"40\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"373\" x=\"320\" y=\"112\" width=\"39\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"374\" x=\"223\" y=\"280\" width=\"36\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"26\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"375\" x=\"340\" y=\"672\" width=\"29\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"19\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"376\" x=\"468\" y=\"224\" width=\"37\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"377\" x=\"600\" y=\"336\" width=\"34\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"378\" x=\"956\" y=\"504\" width=\"30\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"379\" x=\"460\" y=\"336\" width=\"34\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"380\" x=\"987\" y=\"504\" width=\"30\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"381\" x=\"425\" y=\"336\" width=\"34\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"382\" x=\"0\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"383\" x=\"0\" y=\"896\" width=\"20\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"8\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"384\" x=\"292\" y=\"504\" width=\"31\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"385\" x=\"80\" y=\"112\" width=\"39\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"386\" x=\"242\" y=\"392\" width=\"33\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"26\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"387\" x=\"760\" y=\"672\" width=\"29\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"388\" x=\"334\" y=\"280\" width=\"36\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"26\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"389\" x=\"813\" y=\"448\" width=\"32\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"390\" x=\"734\" y=\"224\" width=\"37\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"391\" x=\"396\" y=\"56\" width=\"42\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"392\" x=\"208\" y=\"392\" width=\"33\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"393\" x=\"720\" y=\"168\" width=\"39\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"394\" x=\"734\" y=\"56\" width=\"41\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"32\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"395\" x=\"680\" y=\"448\" width=\"33\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"26\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"396\" x=\"490\" y=\"672\" width=\"29\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"397\" x=\"31\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"398\" x=\"180\" y=\"336\" width=\"34\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"399\" x=\"312\" y=\"224\" width=\"38\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"400\" x=\"846\" y=\"448\" width=\"32\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"401\" x=\"590\" y=\"280\" width=\"35\" height=\"55\" xoffset=\"-7\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"402\" x=\"978\" y=\"448\" width=\"32\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"403\" x=\"177\" y=\"56\" width=\"43\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"404\" x=\"770\" y=\"280\" width=\"35\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"405\" x=\"524\" y=\"56\" width=\"41\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"35\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"406\" x=\"591\" y=\"896\" width=\"15\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"9\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"407\" x=\"831\" y=\"840\" width=\"21\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"11\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"408\" x=\"698\" y=\"280\" width=\"35\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"409\" x=\"786\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"410\" x=\"186\" y=\"896\" width=\"19\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"9\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"411\" x=\"62\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"412\" x=\"482\" y=\"56\" width=\"41\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"36\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"413\" x=\"117\" y=\"224\" width=\"38\" height=\"55\" xoffset=\"-7\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"414\" x=\"206\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"415\" x=\"360\" y=\"168\" width=\"39\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"416\" x=\"221\" y=\"56\" width=\"43\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"34\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"417\" x=\"215\" y=\"336\" width=\"34\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"26\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"418\" x=\"776\" y=\"56\" width=\"41\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"35\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"419\" x=\"250\" y=\"336\" width=\"34\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"420\" x=\"560\" y=\"168\" width=\"39\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"421\" x=\"640\" y=\"672\" width=\"29\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"422\" x=\"734\" y=\"280\" width=\"35\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"423\" x=\"806\" y=\"280\" width=\"35\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"424\" x=\"438\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"425\" x=\"66\" y=\"504\" width=\"32\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"426\" x=\"175\" y=\"840\" width=\"24\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"15\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"427\" x=\"853\" y=\"840\" width=\"21\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"11\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"428\" x=\"495\" y=\"336\" width=\"34\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"429\" x=\"274\" y=\"840\" width=\"23\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"11\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"430\" x=\"915\" y=\"336\" width=\"34\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"431\" x=\"608\" y=\"56\" width=\"41\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"34\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"432\" x=\"0\" y=\"392\" width=\"34\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"433\" x=\"39\" y=\"224\" width=\"38\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"434\" x=\"35\" y=\"392\" width=\"34\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"435\" x=\"650\" y=\"56\" width=\"41\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"436\" x=\"429\" y=\"224\" width=\"38\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"437\" x=\"285\" y=\"336\" width=\"34\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"438\" x=\"93\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"439\" x=\"985\" y=\"336\" width=\"34\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"440\" x=\"880\" y=\"336\" width=\"34\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"441\" x=\"124\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"442\" x=\"155\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"443\" x=\"388\" y=\"504\" width=\"31\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"444\" x=\"186\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"445\" x=\"504\" y=\"784\" width=\"27\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"18\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"446\" x=\"264\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"19\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"447\" x=\"700\" y=\"672\" width=\"29\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"448\" x=\"655\" y=\"896\" width=\"14\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"10\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"449\" x=\"897\" y=\"840\" width=\"21\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"17\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"450\" x=\"228\" y=\"504\" width=\"31\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"23\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"451\" x=\"429\" y=\"896\" width=\"16\" height=\"55\" xoffset=\"-1\" yoffset=\"-5\" xadvance=\"14\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"452\" x=\"0\" y=\"0\" width=\"61\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"53\" page=\"0\" chnl=\"15\" />												"
	<< "	<char id=\"453\" x=\"124\" y=\"0\" width=\"57\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"49\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"454\" x=\"295\" y=\"0\" width=\"51\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"42\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"455\" x=\"704\" y=\"0\" width=\"48\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"42\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"456\" x=\"480\" y=\"168\" width=\"39\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"33\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"457\" x=\"200\" y=\"840\" width=\"24\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"18\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"458\" x=\"240\" y=\"0\" width=\"54\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"49\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"459\" x=\"133\" y=\"56\" width=\"43\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"38\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"460\" x=\"658\" y=\"224\" width=\"37\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"461\" x=\"240\" y=\"168\" width=\"39\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"462\" x=\"217\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"463\" x=\"417\" y=\"840\" width=\"22\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"10\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"464\" x=\"394\" y=\"840\" width=\"22\" height=\"55\" xoffset=\"-7\" yoffset=\"-5\" xadvance=\"8\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"465\" x=\"200\" y=\"168\" width=\"39\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"466\" x=\"248\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"467\" x=\"548\" y=\"392\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"468\" x=\"112\" y=\"784\" width=\"27\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"469\" x=\"582\" y=\"392\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"470\" x=\"140\" y=\"784\" width=\"27\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"471\" x=\"684\" y=\"392\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"472\" x=\"196\" y=\"784\" width=\"27\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"473\" x=\"102\" y=\"448\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"474\" x=\"224\" y=\"784\" width=\"27\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"475\" x=\"136\" y=\"448\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"476\" x=\"252\" y=\"784\" width=\"27\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"477\" x=\"279\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"478\" x=\"920\" y=\"112\" width=\"39\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"479\" x=\"310\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"480\" x=\"840\" y=\"112\" width=\"39\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"481\" x=\"341\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"482\" x=\"450\" y=\"0\" width=\"50\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"40\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"483\" x=\"942\" y=\"0\" width=\"44\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"36\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"484\" x=\"901\" y=\"56\" width=\"40\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"485\" x=\"676\" y=\"504\" width=\"31\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"486\" x=\"390\" y=\"224\" width=\"38\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"487\" x=\"670\" y=\"672\" width=\"29\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"488\" x=\"144\" y=\"336\" width=\"35\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"489\" x=\"728\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"490\" x=\"400\" y=\"112\" width=\"39\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"491\" x=\"372\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"492\" x=\"360\" y=\"112\" width=\"39\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"493\" x=\"403\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"494\" x=\"775\" y=\"336\" width=\"34\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"495\" x=\"434\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"496\" x=\"990\" y=\"784\" width=\"24\" height=\"55\" xoffset=\"-8\" yoffset=\"-5\" xadvance=\"9\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"497\" x=\"62\" y=\"0\" width=\"61\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"53\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"498\" x=\"182\" y=\"0\" width=\"57\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"49\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"499\" x=\"347\" y=\"0\" width=\"51\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"42\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"500\" x=\"195\" y=\"224\" width=\"38\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"501\" x=\"310\" y=\"672\" width=\"29\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"502\" x=\"802\" y=\"0\" width=\"47\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"41\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"503\" x=\"714\" y=\"448\" width=\"32\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"504\" x=\"650\" y=\"392\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"505\" x=\"700\" y=\"784\" width=\"27\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"506\" x=\"40\" y=\"112\" width=\"39\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"507\" x=\"465\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"508\" x=\"399\" y=\"0\" width=\"50\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"40\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"509\" x=\"897\" y=\"0\" width=\"44\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"36\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"510\" x=\"280\" y=\"168\" width=\"39\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"511\" x=\"0\" y=\"728\" width=\"29\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"23\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"512\" x=\"160\" y=\"168\" width=\"39\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"513\" x=\"558\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"514\" x=\"120\" y=\"168\" width=\"39\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"515\" x=\"620\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"516\" x=\"616\" y=\"392\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"517\" x=\"682\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"518\" x=\"204\" y=\"448\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"519\" x=\"744\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"520\" x=\"150\" y=\"840\" width=\"24\" height=\"55\" xoffset=\"-8\" yoffset=\"-5\" xadvance=\"10\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"521\" x=\"887\" y=\"784\" width=\"25\" height=\"55\" xoffset=\"-9\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"522\" x=\"624\" y=\"840\" width=\"22\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"10\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"523\" x=\"125\" y=\"840\" width=\"24\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"524\" x=\"960\" y=\"168\" width=\"39\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"525\" x=\"806\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"526\" x=\"880\" y=\"112\" width=\"39\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"527\" x=\"837\" y=\"560\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"528\" x=\"297\" y=\"280\" width=\"36\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"529\" x=\"809\" y=\"784\" width=\"25\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"13\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"530\" x=\"260\" y=\"280\" width=\"36\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"531\" x=\"532\" y=\"840\" width=\"22\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"13\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"532\" x=\"786\" y=\"392\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"533\" x=\"644\" y=\"784\" width=\"27\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"534\" x=\"820\" y=\"392\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"535\" x=\"588\" y=\"784\" width=\"27\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"536\" x=\"950\" y=\"280\" width=\"35\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"537\" x=\"293\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"19\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"538\" x=\"530\" y=\"336\" width=\"34\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"539\" x=\"716\" y=\"840\" width=\"22\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"11\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"540\" x=\"31\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"541\" x=\"782\" y=\"784\" width=\"26\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"17\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"542\" x=\"922\" y=\"392\" width=\"33\" height=\"55\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"543\" x=\"420\" y=\"784\" width=\"27\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"544\" x=\"565\" y=\"336\" width=\"34\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"28\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"545\" x=\"544\" y=\"224\" width=\"37\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"546\" x=\"747\" y=\"448\" width=\"32\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"547\" x=\"196\" y=\"504\" width=\"31\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"23\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"548\" x=\"635\" y=\"336\" width=\"34\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"549\" x=\"248\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"550\" x=\"40\" y=\"168\" width=\"39\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"551\" x=\"310\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"552\" x=\"670\" y=\"336\" width=\"34\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"553\" x=\"372\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"554\" x=\"0\" y=\"168\" width=\"39\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"555\" x=\"434\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"556\" x=\"960\" y=\"112\" width=\"39\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"557\" x=\"496\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"558\" x=\"800\" y=\"112\" width=\"39\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"559\" x=\"558\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"560\" x=\"80\" y=\"168\" width=\"39\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"561\" x=\"620\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"562\" x=\"186\" y=\"280\" width=\"36\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"26\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"563\" x=\"550\" y=\"672\" width=\"29\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"19\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"564\" x=\"280\" y=\"784\" width=\"27\" height=\"55\" xoffset=\"-7\" yoffset=\"-5\" xadvance=\"14\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"565\" x=\"149\" y=\"280\" width=\"36\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"566\" x=\"56\" y=\"784\" width=\"27\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"15\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"567\" x=\"21\" y=\"896\" width=\"20\" height=\"55\" xoffset=\"-8\" yoffset=\"-5\" xadvance=\"9\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"568\" x=\"89\" y=\"56\" width=\"43\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"35\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"569\" x=\"265\" y=\"56\" width=\"43\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"35\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"570\" x=\"600\" y=\"168\" width=\"39\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"571\" x=\"924\" y=\"224\" width=\"37\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"572\" x=\"837\" y=\"616\" width=\"30\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"573\" x=\"0\" y=\"504\" width=\"32\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"574\" x=\"950\" y=\"336\" width=\"34\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"575\" x=\"496\" y=\"728\" width=\"28\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"576\" x=\"452\" y=\"504\" width=\"31\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"577\" x=\"33\" y=\"504\" width=\"32\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"23\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"578\" x=\"930\" y=\"728\" width=\"27\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"18\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"579\" x=\"445\" y=\"280\" width=\"36\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"580\" x=\"680\" y=\"112\" width=\"39\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"581\" x=\"0\" y=\"280\" width=\"37\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"582\" x=\"70\" y=\"392\" width=\"34\" height=\"55\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"583\" x=\"93\" y=\"672\" width=\"30\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"584\" x=\"580\" y=\"504\" width=\"31\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"585\" x=\"739\" y=\"840\" width=\"22\" height=\"55\" xoffset=\"-8\" yoffset=\"-5\" xadvance=\"9\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"586\" x=\"692\" y=\"56\" width=\"41\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"587\" x=\"646\" y=\"448\" width=\"33\" height=\"55\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"588\" x=\"860\" y=\"56\" width=\"40\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"589\" x=\"835\" y=\"784\" width=\"25\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"13\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"590\" x=\"234\" y=\"224\" width=\"38\" height=\"55\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />											"
	<< "	<char id=\"591\" x=\"279\" y=\"672\" width=\"30\" height=\"55\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />											"
	<< "	</chars>																																										"
	<< "	<kernings count=\"91\">																																							"
	<< "	<kerning first=\"32\" second=\"65\" amount=\"-2\" />																															"
	<< "	<kerning first=\"32\" second=\"84\" amount=\"-1\" />																															"
	<< "	<kerning first=\"32\" second=\"89\" amount=\"-1\" />																															"
	<< "	<kerning first=\"121\" second=\"46\" amount=\"-3\" />																															"
	<< "	<kerning first=\"121\" second=\"44\" amount=\"-3\" />																															"
	<< "	<kerning first=\"119\" second=\"46\" amount=\"-2\" />																															"
	<< "	<kerning first=\"119\" second=\"44\" amount=\"-2\" />																															"
	<< "	<kerning first=\"118\" second=\"46\" amount=\"-3\" />																															"
	<< "	<kerning first=\"118\" second=\"44\" amount=\"-3\" />																															"
	<< "	<kerning first=\"114\" second=\"46\" amount=\"-2\" />																															"
	<< "	<kerning first=\"49\" second=\"49\" amount=\"-3\" />																															"
	<< "	<kerning first=\"65\" second=\"32\" amount=\"-2\" />																															"
	<< "	<kerning first=\"65\" second=\"84\" amount=\"-3\" />																															"
	<< "	<kerning first=\"65\" second=\"86\" amount=\"-3\" />																															"
	<< "	<kerning first=\"65\" second=\"87\" amount=\"-1\" />																															"
	<< "	<kerning first=\"65\" second=\"89\" amount=\"-3\" />																															"
	<< "	<kerning first=\"65\" second=\"118\" amount=\"-1\" />																															"
	<< "	<kerning first=\"65\" second=\"119\" amount=\"-1\" />																															"
	<< "	<kerning first=\"65\" second=\"121\" amount=\"-1\" />																															"
	<< "	<kerning first=\"114\" second=\"44\" amount=\"-2\" />																															"
	<< "	<kerning first=\"70\" second=\"44\" amount=\"-4\" />																															"
	<< "	<kerning first=\"70\" second=\"46\" amount=\"-4\" />																															"
	<< "	<kerning first=\"70\" second=\"65\" amount=\"-2\" />																															"
	<< "	<kerning first=\"76\" second=\"32\" amount=\"-1\" />																															"
	<< "	<kerning first=\"76\" second=\"84\" amount=\"-3\" />																															"
	<< "	<kerning first=\"76\" second=\"86\" amount=\"-3\" />																															"
	<< "	<kerning first=\"76\" second=\"87\" amount=\"-3\" />																															"
	<< "	<kerning first=\"76\" second=\"89\" amount=\"-3\" />																															"
	<< "	<kerning first=\"76\" second=\"121\" amount=\"-1\" />																															"
	<< "	<kerning first=\"102\" second=\"102\" amount=\"-1\" />																															"
	<< "	<kerning first=\"80\" second=\"32\" amount=\"-1\" />																															"
	<< "	<kerning first=\"80\" second=\"44\" amount=\"-5\" />																															"
	<< "	<kerning first=\"80\" second=\"46\" amount=\"-5\" />																															"
	<< "	<kerning first=\"80\" second=\"65\" amount=\"-3\" />																															"
	<< "	<kerning first=\"82\" second=\"84\" amount=\"-1\" />																															"
	<< "	<kerning first=\"82\" second=\"86\" amount=\"-1\" />																															"
	<< "	<kerning first=\"82\" second=\"87\" amount=\"-1\" />																															"
	<< "	<kerning first=\"82\" second=\"89\" amount=\"-1\" />																															"
	<< "	<kerning first=\"84\" second=\"32\" amount=\"-1\" />																															"
	<< "	<kerning first=\"84\" second=\"44\" amount=\"-4\" />																															"
	<< "	<kerning first=\"84\" second=\"45\" amount=\"-2\" />																															"
	<< "	<kerning first=\"84\" second=\"46\" amount=\"-4\" />																															"
	<< "	<kerning first=\"84\" second=\"58\" amount=\"-4\" />																															"
	<< "	<kerning first=\"89\" second=\"118\" amount=\"-2\" />																															"
	<< "	<kerning first=\"84\" second=\"65\" amount=\"-3\" />																															"
	<< "	<kerning first=\"84\" second=\"79\" amount=\"-1\" />																															"
	<< "	<kerning first=\"84\" second=\"97\" amount=\"-4\" />																															"
	<< "	<kerning first=\"84\" second=\"99\" amount=\"-4\" />																															"
	<< "	<kerning first=\"84\" second=\"101\" amount=\"-4\" />																															"
	<< "	<kerning first=\"84\" second=\"105\" amount=\"-1\" />																															"
	<< "	<kerning first=\"84\" second=\"111\" amount=\"-4\" />																															"
	<< "	<kerning first=\"84\" second=\"114\" amount=\"-1\" />																															"
	<< "	<kerning first=\"84\" second=\"115\" amount=\"-4\" />																															"
	<< "	<kerning first=\"84\" second=\"117\" amount=\"-1\" />																															"
	<< "	<kerning first=\"84\" second=\"119\" amount=\"-2\" />																															"
	<< "	<kerning first=\"84\" second=\"121\" amount=\"-2\" />																															"
	<< "	<kerning first=\"86\" second=\"44\" amount=\"-4\" />																															"
	<< "	<kerning first=\"86\" second=\"45\" amount=\"-2\" />																															"
	<< "	<kerning first=\"86\" second=\"46\" amount=\"-4\" />																															"
	<< "	<kerning first=\"86\" second=\"58\" amount=\"-1\" />																															"
	<< "	<kerning first=\"89\" second=\"117\" amount=\"-2\" />																															"
	<< "	<kerning first=\"86\" second=\"65\" amount=\"-3\" />																															"
	<< "	<kerning first=\"86\" second=\"97\" amount=\"-3\" />																															"
	<< "	<kerning first=\"86\" second=\"101\" amount=\"-2\" />																															"
	<< "	<kerning first=\"86\" second=\"105\" amount=\"-1\" />																															"
	<< "	<kerning first=\"86\" second=\"111\" amount=\"-2\" />																															"
	<< "	<kerning first=\"86\" second=\"114\" amount=\"-1\" />																															"
	<< "	<kerning first=\"86\" second=\"117\" amount=\"-1\" />																															"
	<< "	<kerning first=\"86\" second=\"121\" amount=\"-1\" />																															"
	<< "	<kerning first=\"87\" second=\"44\" amount=\"-2\" />																															"
	<< "	<kerning first=\"87\" second=\"45\" amount=\"-1\" />																															"
	<< "	<kerning first=\"87\" second=\"46\" amount=\"-2\" />																															"
	<< "	<kerning first=\"87\" second=\"58\" amount=\"-1\" />																															"
	<< "	<kerning first=\"89\" second=\"113\" amount=\"-4\" />																															"
	<< "	<kerning first=\"87\" second=\"65\" amount=\"-1\" />																															"
	<< "	<kerning first=\"87\" second=\"97\" amount=\"-1\" />																															"
	<< "	<kerning first=\"87\" second=\"101\" amount=\"-1\" />																															"
	<< "	<kerning first=\"89\" second=\"112\" amount=\"-3\" />																															"
	<< "	<kerning first=\"87\" second=\"111\" amount=\"-1\" />																															"
	<< "	<kerning first=\"87\" second=\"114\" amount=\"-1\" />																															"
	<< "	<kerning first=\"87\" second=\"117\" amount=\"-1\" />																															"
	<< "	<kerning first=\"89\" second=\"111\" amount=\"-4\" />																															"
	<< "	<kerning first=\"89\" second=\"32\" amount=\"-1\" />																															"
	<< "	<kerning first=\"89\" second=\"44\" amount=\"-5\" />																															"
	<< "	<kerning first=\"89\" second=\"45\" amount=\"-4\" />																															"
	<< "	<kerning first=\"89\" second=\"46\" amount=\"-5\" />																															"
	<< "	<kerning first=\"89\" second=\"58\" amount=\"-2\" />																															"
	<< "	<kerning first=\"89\" second=\"105\" amount=\"-1\" />																															"
	<< "	<kerning first=\"89\" second=\"65\" amount=\"-3\" />																															"
	<< "	<kerning first=\"89\" second=\"97\" amount=\"-3\" />																															"
	<< "	<kerning first=\"89\" second=\"101\" amount=\"-4\" />																															"
	<< "	</kernings>																																										"
	<< "	</font>																																											";
#pragma endregion Arial_Xml

#pragma region Arial_Normal_Xml
	m_buildInFontXML[ARIAL_NORMAL]
	<< "	<?xml version=\"1.0\"?>																																											"
	<< "	<font>																																															"
	<< "	<info face=\"Arial\" size=\"-64\" bold=\"0\" italic=\"0\" charset=\"\" unicode=\"1\" stretchH=\"100\" smooth=\"1\" aa=\"1\" padding=\"0,0,0,0\" spacing=\"1,1\" outline=\"0\"/>					"
	<< "	<common lineHeight=\"72\" base=\"58\" scaleW=\"1024\" scaleH=\"1024\" pages=\"1\" packed=\"0\" alphaChnl=\"1\" redChnl=\"0\" greenChnl=\"0\" blueChnl=\"0\"/>									"
	<< "	<pages>																																															"
	<< "	<page id=\"0\" file=\"arial_normal_0.png\" />																																					"
	<< "	</pages>																																														"
	<< "	<chars count=\"331\">																																											"
	<< "	<char id=\"32\" x=\"609\" y=\"730\" width=\"3\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"33\" x=\"507\" y=\"730\" width=\"8\" height=\"72\" xoffset=\"6\" yoffset=\"0\" xadvance=\"20\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"34\" x=\"100\" y=\"730\" width=\"19\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"23\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"35\" x=\"966\" y=\"292\" width=\"36\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"36\" x=\"204\" y=\"438\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"37\" x=\"599\" y=\"0\" width=\"51\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"57\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"38\" x=\"169\" y=\"219\" width=\"40\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"39\" x=\"534\" y=\"730\" width=\"8\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"12\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"40\" x=\"230\" y=\"730\" width=\"16\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"41\" x=\"195\" y=\"730\" width=\"17\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"42\" x=\"525\" y=\"657\" width=\"23\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"25\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"43\" x=\"237\" y=\"438\" width=\"32\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"37\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"44\" x=\"498\" y=\"730\" width=\"8\" height=\"72\" xoffset=\"5\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"45\" x=\"40\" y=\"730\" width=\"19\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"21\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"46\" x=\"543\" y=\"730\" width=\"8\" height=\"72\" xoffset=\"5\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"47\" x=\"813\" y=\"657\" width=\"20\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"48\" x=\"329\" y=\"511\" width=\"31\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"49\" x=\"938\" y=\"657\" width=\"19\" height=\"72\" xoffset=\"6\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"50\" x=\"336\" y=\"438\" width=\"32\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"51\" x=\"369\" y=\"438\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"52\" x=\"623\" y=\"365\" width=\"34\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"53\" x=\"567\" y=\"438\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"54\" x=\"765\" y=\"438\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"55\" x=\"402\" y=\"438\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"56\" x=\"435\" y=\"438\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"57\" x=\"132\" y=\"511\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"58\" x=\"552\" y=\"730\" width=\"8\" height=\"72\" xoffset=\"5\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"59\" x=\"525\" y=\"730\" width=\"8\" height=\"72\" xoffset=\"5\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"60\" x=\"393\" y=\"511\" width=\"31\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"37\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"61\" x=\"425\" y=\"511\" width=\"31\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"37\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"62\" x=\"937\" y=\"511\" width=\"31\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"37\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"63\" x=\"905\" y=\"511\" width=\"31\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"64\" x=\"256\" y=\"0\" width=\"60\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"65\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"65\" x=\"360\" y=\"146\" width=\"44\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"66\" x=\"74\" y=\"365\" width=\"36\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"67\" x=\"537\" y=\"146\" width=\"42\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"68\" x=\"250\" y=\"219\" width=\"39\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"69\" x=\"37\" y=\"365\" width=\"36\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"70\" x=\"761\" y=\"365\" width=\"33\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"39\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"71\" x=\"225\" y=\"146\" width=\"44\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"50\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"72\" x=\"351\" y=\"292\" width=\"38\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"73\" x=\"516\" y=\"730\" width=\"8\" height=\"72\" xoffset=\"5\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"74\" x=\"996\" y=\"438\" width=\"27\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"75\" x=\"210\" y=\"219\" width=\"39\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"76\" x=\"188\" y=\"584\" width=\"30\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"77\" x=\"704\" y=\"73\" width=\"45\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"53\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"78\" x=\"390\" y=\"292\" width=\"38\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"79\" x=\"564\" y=\"73\" width=\"46\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"50\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"80\" x=\"777\" y=\"292\" width=\"37\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"81\" x=\"517\" y=\"73\" width=\"46\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"50\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"82\" x=\"881\" y=\"146\" width=\"42\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"83\" x=\"429\" y=\"292\" width=\"38\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"84\" x=\"468\" y=\"292\" width=\"38\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"40\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"85\" x=\"507\" y=\"292\" width=\"38\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"86\" x=\"450\" y=\"146\" width=\"43\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"87\" x=\"65\" y=\"0\" width=\"64\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"64\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"88\" x=\"86\" y=\"219\" width=\"41\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"41\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"89\" x=\"43\" y=\"219\" width=\"42\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"42\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"90\" x=\"891\" y=\"292\" width=\"37\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"39\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"91\" x=\"357\" y=\"730\" width=\"14\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"92\" x=\"792\" y=\"657\" width=\"20\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"93\" x=\"296\" y=\"730\" width=\"15\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"94\" x=\"998\" y=\"0\" width=\"25\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"95\" x=\"490\" y=\"219\" width=\"39\" height=\"72\" xoffset=\"-2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"96\" x=\"386\" y=\"730\" width=\"13\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"97\" x=\"303\" y=\"438\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"98\" x=\"219\" y=\"584\" width=\"30\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"99\" x=\"250\" y=\"584\" width=\"30\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"100\" x=\"281\" y=\"584\" width=\"30\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"101\" x=\"66\" y=\"511\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"102\" x=\"617\" y=\"657\" width=\"21\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"103\" x=\"809\" y=\"511\" width=\"31\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"104\" x=\"269\" y=\"657\" width=\"28\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"105\" x=\"585\" y=\"730\" width=\"7\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"14\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"106\" x=\"342\" y=\"730\" width=\"14\" height=\"72\" xoffset=\"-3\" yoffset=\"0\" xadvance=\"15\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"107\" x=\"981\" y=\"584\" width=\"29\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"33\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"108\" x=\"593\" y=\"730\" width=\"7\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"14\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"109\" x=\"853\" y=\"0\" width=\"48\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"55\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"110\" x=\"298\" y=\"657\" width=\"28\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"111\" x=\"68\" y=\"438\" width=\"33\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"112\" x=\"312\" y=\"584\" width=\"30\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"113\" x=\"343\" y=\"584\" width=\"30\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"114\" x=\"918\" y=\"657\" width=\"19\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"115\" x=\"90\" y=\"657\" width=\"29\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"116\" x=\"158\" y=\"730\" width=\"18\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"117\" x=\"120\" y=\"657\" width=\"29\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"118\" x=\"585\" y=\"511\" width=\"31\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"33\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"119\" x=\"902\" y=\"0\" width=\"47\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"47\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"120\" x=\"521\" y=\"511\" width=\"31\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"31\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"121\" x=\"489\" y=\"511\" width=\"31\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"31\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"122\" x=\"457\" y=\"511\" width=\"31\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"31\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"123\" x=\"0\" y=\"730\" width=\"19\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"21\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"124\" x=\"561\" y=\"730\" width=\"7\" height=\"72\" xoffset=\"5\" yoffset=\"0\" xadvance=\"17\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"125\" x=\"771\" y=\"657\" width=\"20\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"126\" x=\"34\" y=\"438\" width=\"33\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"37\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"160\" x=\"1018\" y=\"657\" width=\"3\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"161\" x=\"489\" y=\"730\" width=\"8\" height=\"72\" xoffset=\"6\" yoffset=\"0\" xadvance=\"20\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"162\" x=\"374\" y=\"584\" width=\"30\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"163\" x=\"517\" y=\"365\" width=\"35\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"164\" x=\"468\" y=\"438\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"165\" x=\"296\" y=\"365\" width=\"36\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"166\" x=\"601\" y=\"730\" width=\"7\" height=\"72\" xoffset=\"5\" yoffset=\"0\" xadvance=\"17\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"167\" x=\"699\" y=\"438\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"168\" x=\"958\" y=\"657\" width=\"19\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"169\" x=\"803\" y=\"0\" width=\"49\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"47\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"170\" x=\"572\" y=\"657\" width=\"22\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"24\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"171\" x=\"443\" y=\"657\" width=\"28\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"172\" x=\"0\" y=\"584\" width=\"31\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"37\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"173\" x=\"20\" y=\"730\" width=\"19\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"174\" x=\"753\" y=\"0\" width=\"49\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"47\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"175\" x=\"410\" y=\"219\" width=\"39\" height=\"72\" xoffset=\"-2\" yoffset=\"0\" xadvance=\"35\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"176\" x=\"998\" y=\"657\" width=\"19\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"26\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"177\" x=\"99\" y=\"511\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"35\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"178\" x=\"727\" y=\"657\" width=\"21\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"179\" x=\"595\" y=\"657\" width=\"21\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"180\" x=\"400\" y=\"730\" width=\"13\" height=\"72\" xoffset=\"6\" yoffset=\"0\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"181\" x=\"651\" y=\"584\" width=\"29\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"37\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"182\" x=\"333\" y=\"365\" width=\"36\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"34\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"183\" x=\"577\" y=\"730\" width=\"7\" height=\"72\" xoffset=\"7\" yoffset=\"0\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"184\" x=\"264\" y=\"730\" width=\"15\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"185\" x=\"1010\" y=\"146\" width=\"13\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"186\" x=\"749\" y=\"657\" width=\"21\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"23\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"187\" x=\"414\" y=\"657\" width=\"28\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"188\" x=\"651\" y=\"0\" width=\"50\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"53\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"189\" x=\"702\" y=\"0\" width=\"50\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"53\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"190\" x=\"545\" y=\"0\" width=\"53\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"53\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"191\" x=\"553\" y=\"511\" width=\"31\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"39\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"192\" x=\"888\" y=\"73\" width=\"45\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"193\" x=\"842\" y=\"73\" width=\"45\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"194\" x=\"796\" y=\"73\" width=\"45\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"195\" x=\"750\" y=\"73\" width=\"45\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"196\" x=\"658\" y=\"73\" width=\"45\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"197\" x=\"405\" y=\"146\" width=\"44\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"198\" x=\"130\" y=\"0\" width=\"62\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"64\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"199\" x=\"0\" y=\"219\" width=\"42\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"200\" x=\"111\" y=\"365\" width=\"36\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"201\" x=\"148\" y=\"365\" width=\"36\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"202\" x=\"185\" y=\"365\" width=\"36\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"203\" x=\"222\" y=\"365\" width=\"36\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"204\" x=\"1011\" y=\"584\" width=\"12\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"205\" x=\"453\" y=\"730\" width=\"12\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"206\" x=\"683\" y=\"657\" width=\"21\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"207\" x=\"1003\" y=\"292\" width=\"20\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"208\" x=\"934\" y=\"73\" width=\"44\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"209\" x=\"585\" y=\"292\" width=\"38\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"210\" x=\"141\" y=\"73\" width=\"46\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"50\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"211\" x=\"0\" y=\"73\" width=\"46\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"50\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"212\" x=\"47\" y=\"73\" width=\"46\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"50\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"213\" x=\"94\" y=\"73\" width=\"46\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"50\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"214\" x=\"188\" y=\"73\" width=\"46\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"50\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"215\" x=\"801\" y=\"584\" width=\"29\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"37\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"216\" x=\"282\" y=\"73\" width=\"46\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"50\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"217\" x=\"624\" y=\"292\" width=\"38\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"218\" x=\"569\" y=\"219\" width=\"38\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"219\" x=\"608\" y=\"219\" width=\"38\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"220\" x=\"647\" y=\"219\" width=\"38\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"221\" x=\"967\" y=\"146\" width=\"42\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"222\" x=\"853\" y=\"292\" width=\"37\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"223\" x=\"553\" y=\"365\" width=\"34\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"39\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"224\" x=\"270\" y=\"438\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"225\" x=\"501\" y=\"438\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"226\" x=\"600\" y=\"438\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"227\" x=\"831\" y=\"438\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"228\" x=\"798\" y=\"438\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"229\" x=\"732\" y=\"438\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"230\" x=\"435\" y=\"0\" width=\"54\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"57\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"231\" x=\"436\" y=\"584\" width=\"30\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"232\" x=\"264\" y=\"511\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"233\" x=\"0\" y=\"511\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"234\" x=\"963\" y=\"438\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"235\" x=\"930\" y=\"438\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"236\" x=\"372\" y=\"730\" width=\"13\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"237\" x=\"414\" y=\"730\" width=\"12\" height=\"72\" xoffset=\"6\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"238\" x=\"705\" y=\"657\" width=\"21\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"239\" x=\"855\" y=\"657\" width=\"20\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"240\" x=\"897\" y=\"438\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"241\" x=\"385\" y=\"657\" width=\"28\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"242\" x=\"0\" y=\"438\" width=\"33\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"243\" x=\"863\" y=\"365\" width=\"33\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"244\" x=\"693\" y=\"365\" width=\"33\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"245\" x=\"170\" y=\"438\" width=\"33\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"246\" x=\"136\" y=\"438\" width=\"33\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"247\" x=\"873\" y=\"511\" width=\"31\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"35\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"248\" x=\"102\" y=\"438\" width=\"33\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"39\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"249\" x=\"711\" y=\"584\" width=\"29\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"250\" x=\"0\" y=\"657\" width=\"29\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"251\" x=\"30\" y=\"657\" width=\"29\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"252\" x=\"681\" y=\"584\" width=\"29\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"253\" x=\"713\" y=\"511\" width=\"31\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"254\" x=\"467\" y=\"584\" width=\"30\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"255\" x=\"969\" y=\"511\" width=\"31\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"256\" x=\"315\" y=\"146\" width=\"44\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"257\" x=\"666\" y=\"438\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"258\" x=\"90\" y=\"146\" width=\"44\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"259\" x=\"231\" y=\"511\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"260\" x=\"470\" y=\"73\" width=\"46\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"261\" x=\"658\" y=\"365\" width=\"34\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"262\" x=\"924\" y=\"146\" width=\"42\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"263\" x=\"498\" y=\"584\" width=\"30\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"264\" x=\"623\" y=\"146\" width=\"42\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"265\" x=\"529\" y=\"584\" width=\"30\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"266\" x=\"709\" y=\"146\" width=\"42\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"267\" x=\"560\" y=\"584\" width=\"30\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"268\" x=\"752\" y=\"146\" width=\"42\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"269\" x=\"64\" y=\"584\" width=\"30\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"270\" x=\"370\" y=\"219\" width=\"39\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"271\" x=\"290\" y=\"219\" width=\"39\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"39\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"272\" x=\"0\" y=\"146\" width=\"44\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"273\" x=\"481\" y=\"365\" width=\"35\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"274\" x=\"407\" y=\"365\" width=\"36\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"275\" x=\"864\" y=\"438\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"276\" x=\"0\" y=\"365\" width=\"36\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"277\" x=\"33\" y=\"511\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"278\" x=\"259\" y=\"365\" width=\"36\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"279\" x=\"534\" y=\"438\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"280\" x=\"701\" y=\"292\" width=\"37\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"281\" x=\"198\" y=\"511\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"282\" x=\"370\" y=\"365\" width=\"36\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"283\" x=\"165\" y=\"511\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"284\" x=\"180\" y=\"146\" width=\"44\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"50\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"285\" x=\"745\" y=\"511\" width=\"31\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"286\" x=\"979\" y=\"73\" width=\"44\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"50\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"287\" x=\"777\" y=\"511\" width=\"31\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"288\" x=\"270\" y=\"146\" width=\"44\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"50\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"289\" x=\"361\" y=\"511\" width=\"31\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"290\" x=\"135\" y=\"146\" width=\"44\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"50\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"291\" x=\"297\" y=\"511\" width=\"31\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"292\" x=\"842\" y=\"219\" width=\"38\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"293\" x=\"240\" y=\"657\" width=\"28\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"294\" x=\"235\" y=\"73\" width=\"46\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"295\" x=\"795\" y=\"365\" width=\"33\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"296\" x=\"999\" y=\"365\" width=\"23\" height=\"72\" xoffset=\"-2\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"297\" x=\"501\" y=\"657\" width=\"23\" height=\"72\" xoffset=\"-2\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"298\" x=\"639\" y=\"657\" width=\"21\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"299\" x=\"661\" y=\"657\" width=\"21\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"300\" x=\"876\" y=\"657\" width=\"20\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"301\" x=\"834\" y=\"657\" width=\"20\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"302\" x=\"440\" y=\"730\" width=\"12\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"303\" x=\"427\" y=\"730\" width=\"12\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"14\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"304\" x=\"479\" y=\"730\" width=\"9\" height=\"72\" xoffset=\"5\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"305\" x=\"569\" y=\"730\" width=\"7\" height=\"72\" xoffset=\"6\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"306\" x=\"128\" y=\"219\" width=\"40\" height=\"72\" xoffset=\"5\" yoffset=\"0\" xadvance=\"47\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"307\" x=\"1001\" y=\"511\" width=\"22\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"28\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"308\" x=\"727\" y=\"365\" width=\"33\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"309\" x=\"549\" y=\"657\" width=\"22\" height=\"72\" xoffset=\"-4\" yoffset=\"0\" xadvance=\"15\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"310\" x=\"450\" y=\"219\" width=\"39\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"311\" x=\"771\" y=\"584\" width=\"29\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"33\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"312\" x=\"831\" y=\"584\" width=\"29\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"313\" x=\"405\" y=\"584\" width=\"30\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"314\" x=\"466\" y=\"730\" width=\"12\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"14\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"315\" x=\"95\" y=\"584\" width=\"30\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"316\" x=\"327\" y=\"730\" width=\"14\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"14\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"317\" x=\"126\" y=\"584\" width=\"30\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"318\" x=\"213\" y=\"730\" width=\"16\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"19\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"319\" x=\"157\" y=\"584\" width=\"30\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"320\" x=\"139\" y=\"730\" width=\"18\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"321\" x=\"929\" y=\"292\" width=\"36\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"322\" x=\"247\" y=\"730\" width=\"16\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"14\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"323\" x=\"530\" y=\"219\" width=\"38\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"324\" x=\"327\" y=\"657\" width=\"28\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"325\" x=\"546\" y=\"292\" width=\"38\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"326\" x=\"356\" y=\"657\" width=\"28\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"327\" x=\"725\" y=\"219\" width=\"38\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"328\" x=\"472\" y=\"657\" width=\"28\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"329\" x=\"444\" y=\"365\" width=\"36\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"39\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"330\" x=\"330\" y=\"219\" width=\"39\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"331\" x=\"210\" y=\"657\" width=\"29\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"332\" x=\"329\" y=\"73\" width=\"46\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"50\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"333\" x=\"897\" y=\"365\" width=\"33\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"334\" x=\"423\" y=\"73\" width=\"46\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"50\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"335\" x=\"931\" y=\"365\" width=\"33\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"336\" x=\"376\" y=\"73\" width=\"46\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"50\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"337\" x=\"965\" y=\"365\" width=\"33\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"338\" x=\"317\" y=\"0\" width=\"60\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"64\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"339\" x=\"378\" y=\"0\" width=\"56\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"60\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"340\" x=\"838\" y=\"146\" width=\"42\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"341\" x=\"60\" y=\"730\" width=\"19\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"342\" x=\"795\" y=\"146\" width=\"42\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"343\" x=\"80\" y=\"730\" width=\"19\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"344\" x=\"666\" y=\"146\" width=\"42\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"345\" x=\"897\" y=\"657\" width=\"20\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"346\" x=\"764\" y=\"219\" width=\"38\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"347\" x=\"591\" y=\"584\" width=\"29\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"348\" x=\"803\" y=\"219\" width=\"38\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"349\" x=\"180\" y=\"657\" width=\"29\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"350\" x=\"881\" y=\"219\" width=\"38\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"351\" x=\"150\" y=\"657\" width=\"29\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"352\" x=\"920\" y=\"219\" width=\"38\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"353\" x=\"60\" y=\"657\" width=\"29\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"354\" x=\"686\" y=\"219\" width=\"38\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"40\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"355\" x=\"978\" y=\"657\" width=\"19\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"356\" x=\"959\" y=\"219\" width=\"38\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"40\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"357\" x=\"998\" y=\"219\" width=\"25\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"24\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"358\" x=\"0\" y=\"292\" width=\"38\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"39\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"359\" x=\"177\" y=\"730\" width=\"17\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"360\" x=\"39\" y=\"292\" width=\"38\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"361\" x=\"951\" y=\"584\" width=\"29\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"362\" x=\"78\" y=\"292\" width=\"38\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"363\" x=\"921\" y=\"584\" width=\"29\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"364\" x=\"117\" y=\"292\" width=\"38\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"365\" x=\"891\" y=\"584\" width=\"29\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"366\" x=\"156\" y=\"292\" width=\"38\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"367\" x=\"861\" y=\"584\" width=\"29\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"368\" x=\"195\" y=\"292\" width=\"38\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"369\" x=\"621\" y=\"584\" width=\"29\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"370\" x=\"234\" y=\"292\" width=\"38\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"46\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"371\" x=\"649\" y=\"511\" width=\"31\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"372\" x=\"0\" y=\"0\" width=\"64\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"64\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"373\" x=\"950\" y=\"0\" width=\"47\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"47\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"374\" x=\"494\" y=\"146\" width=\"42\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"42\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"375\" x=\"841\" y=\"511\" width=\"31\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"31\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"376\" x=\"580\" y=\"146\" width=\"42\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"377\" x=\"739\" y=\"292\" width=\"37\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"39\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"378\" x=\"617\" y=\"511\" width=\"31\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"31\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"379\" x=\"815\" y=\"292\" width=\"37\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"39\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"380\" x=\"681\" y=\"511\" width=\"31\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"31\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"381\" x=\"663\" y=\"292\" width=\"37\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"39\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"382\" x=\"32\" y=\"584\" width=\"31\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"31\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"383\" x=\"280\" y=\"730\" width=\"15\" height=\"72\" xoffset=\"4\" yoffset=\"0\" xadvance=\"14\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"402\" x=\"588\" y=\"365\" width=\"34\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"506\" x=\"45\" y=\"146\" width=\"44\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"507\" x=\"633\" y=\"438\" width=\"32\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"508\" x=\"193\" y=\"0\" width=\"62\" height=\"72\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"64\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"509\" x=\"490\" y=\"0\" width=\"54\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"57\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"510\" x=\"611\" y=\"73\" width=\"46\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"50\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"511\" x=\"829\" y=\"365\" width=\"33\" height=\"72\" xoffset=\"3\" yoffset=\"0\" xadvance=\"39\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"536\" x=\"273\" y=\"292\" width=\"38\" height=\"72\" xoffset=\"2\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"537\" x=\"741\" y=\"584\" width=\"29\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"538\" x=\"312\" y=\"292\" width=\"38\" height=\"72\" xoffset=\"1\" yoffset=\"0\" xadvance=\"40\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"539\" x=\"120\" y=\"730\" width=\"18\" height=\"72\" xoffset=\"0\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"567\" x=\"312\" y=\"730\" width=\"14\" height=\"72\" xoffset=\"-4\" yoffset=\"0\" xadvance=\"14\" page=\"0\" chnl=\"15\" />															"
	<< "	</chars>																																														"
	<< "	<kernings count=\"92\">																																											"
	<< "	<kerning first=\"32\" second=\"65\" amount=\"-4\" />																																			"
	<< "	<kerning first=\"32\" second=\"84\" amount=\"-1\" />																																			"
	<< "	<kerning first=\"32\" second=\"89\" amount=\"-1\" />																																			"
	<< "	<kerning first=\"121\" second=\"46\" amount=\"-5\" />																																			"
	<< "	<kerning first=\"121\" second=\"44\" amount=\"-5\" />																																			"
	<< "	<kerning first=\"119\" second=\"46\" amount=\"-4\" />																																			"
	<< "	<kerning first=\"119\" second=\"44\" amount=\"-4\" />																																			"
	<< "	<kerning first=\"118\" second=\"46\" amount=\"-5\" />																																			"
	<< "	<kerning first=\"118\" second=\"44\" amount=\"-5\" />																																			"
	<< "	<kerning first=\"114\" second=\"46\" amount=\"-4\" />																																			"
	<< "	<kerning first=\"49\" second=\"49\" amount=\"-5\" />																																			"
	<< "	<kerning first=\"65\" second=\"32\" amount=\"-4\" />																																			"
	<< "	<kerning first=\"65\" second=\"84\" amount=\"-5\" />																																			"
	<< "	<kerning first=\"65\" second=\"86\" amount=\"-5\" />																																			"
	<< "	<kerning first=\"65\" second=\"87\" amount=\"-2\" />																																			"
	<< "	<kerning first=\"65\" second=\"89\" amount=\"-5\" />																																			"
	<< "	<kerning first=\"65\" second=\"118\" amount=\"-1\" />																																			"
	<< "	<kerning first=\"65\" second=\"119\" amount=\"-1\" />																																			"
	<< "	<kerning first=\"65\" second=\"121\" amount=\"-1\" />																																			"
	<< "	<kerning first=\"114\" second=\"44\" amount=\"-4\" />																																			"
	<< "	<kerning first=\"70\" second=\"44\" amount=\"-7\" />																																			"
	<< "	<kerning first=\"70\" second=\"46\" amount=\"-7\" />																																			"
	<< "	<kerning first=\"70\" second=\"65\" amount=\"-4\" />																																			"
	<< "	<kerning first=\"76\" second=\"32\" amount=\"-2\" />																																			"
	<< "	<kerning first=\"76\" second=\"84\" amount=\"-5\" />																																			"
	<< "	<kerning first=\"76\" second=\"86\" amount=\"-5\" />																																			"
	<< "	<kerning first=\"76\" second=\"87\" amount=\"-5\" />																																			"
	<< "	<kerning first=\"76\" second=\"89\" amount=\"-5\" />																																			"
	<< "	<kerning first=\"76\" second=\"121\" amount=\"-2\" />																																			"
	<< "	<kerning first=\"102\" second=\"102\" amount=\"-1\" />																																			"
	<< "	<kerning first=\"80\" second=\"32\" amount=\"-1\" />																																			"
	<< "	<kerning first=\"80\" second=\"44\" amount=\"-8\" />																																			"
	<< "	<kerning first=\"80\" second=\"46\" amount=\"-8\" />																																			"
	<< "	<kerning first=\"80\" second=\"65\" amount=\"-5\" />																																			"
	<< "	<kerning first=\"82\" second=\"84\" amount=\"-1\" />																																			"
	<< "	<kerning first=\"82\" second=\"86\" amount=\"-1\" />																																			"
	<< "	<kerning first=\"82\" second=\"87\" amount=\"-1\" />																																			"
	<< "	<kerning first=\"82\" second=\"89\" amount=\"-1\" />																																			"
	<< "	<kerning first=\"84\" second=\"32\" amount=\"-1\" />																																			"
	<< "	<kerning first=\"84\" second=\"44\" amount=\"-7\" />																																			"
	<< "	<kerning first=\"84\" second=\"45\" amount=\"-4\" />																																			"
	<< "	<kerning first=\"84\" second=\"46\" amount=\"-7\" />																																			"
	<< "	<kerning first=\"84\" second=\"58\" amount=\"-7\" />																																			"
	<< "	<kerning first=\"89\" second=\"118\" amount=\"-4\" />																																			"
	<< "	<kerning first=\"84\" second=\"65\" amount=\"-5\" />																																			"
	<< "	<kerning first=\"84\" second=\"79\" amount=\"-1\" />																																			"
	<< "	<kerning first=\"84\" second=\"97\" amount=\"-7\" />																																			"
	<< "	<kerning first=\"84\" second=\"99\" amount=\"-7\" />																																			"
	<< "	<kerning first=\"84\" second=\"101\" amount=\"-7\" />																																			"
	<< "	<kerning first=\"84\" second=\"105\" amount=\"-2\" />																																			"
	<< "	<kerning first=\"84\" second=\"111\" amount=\"-7\" />																																			"
	<< "	<kerning first=\"84\" second=\"114\" amount=\"-2\" />																																			"
	<< "	<kerning first=\"84\" second=\"115\" amount=\"-7\" />																																			"
	<< "	<kerning first=\"84\" second=\"117\" amount=\"-2\" />																																			"
	<< "	<kerning first=\"84\" second=\"119\" amount=\"-4\" />																																			"
	<< "	<kerning first=\"84\" second=\"121\" amount=\"-4\" />																																			"
	<< "	<kerning first=\"86\" second=\"44\" amount=\"-6\" />																																			"
	<< "	<kerning first=\"86\" second=\"45\" amount=\"-4\" />																																			"
	<< "	<kerning first=\"86\" second=\"46\" amount=\"-6\" />																																			"
	<< "	<kerning first=\"86\" second=\"58\" amount=\"-2\" />																																			"
	<< "	<kerning first=\"89\" second=\"117\" amount=\"-4\" />																																			"
	<< "	<kerning first=\"86\" second=\"65\" amount=\"-5\" />																																			"
	<< "	<kerning first=\"86\" second=\"97\" amount=\"-5\" />																																			"
	<< "	<kerning first=\"86\" second=\"101\" amount=\"-4\" />																																			"
	<< "	<kerning first=\"86\" second=\"105\" amount=\"-1\" />																																			"
	<< "	<kerning first=\"86\" second=\"111\" amount=\"-4\" />																																			"
	<< "	<kerning first=\"86\" second=\"114\" amount=\"-2\" />																																			"
	<< "	<kerning first=\"86\" second=\"117\" amount=\"-2\" />																																			"
	<< "	<kerning first=\"86\" second=\"121\" amount=\"-2\" />																																			"
	<< "	<kerning first=\"87\" second=\"44\" amount=\"-4\" />																																			"
	<< "	<kerning first=\"87\" second=\"45\" amount=\"-1\" />																																			"
	<< "	<kerning first=\"87\" second=\"46\" amount=\"-4\" />																																			"
	<< "	<kerning first=\"87\" second=\"58\" amount=\"-1\" />																																			"
	<< "	<kerning first=\"89\" second=\"113\" amount=\"-6\" />																																			"
	<< "	<kerning first=\"87\" second=\"65\" amount=\"-2\" />																																			"
	<< "	<kerning first=\"87\" second=\"97\" amount=\"-2\" />																																			"
	<< "	<kerning first=\"87\" second=\"101\" amount=\"-1\" />																																			"
	<< "	<kerning first=\"89\" second=\"112\" amount=\"-5\" />																																			"
	<< "	<kerning first=\"87\" second=\"111\" amount=\"-1\" />																																			"
	<< "	<kerning first=\"87\" second=\"114\" amount=\"-1\" />																																			"
	<< "	<kerning first=\"87\" second=\"117\" amount=\"-1\" />																																			"
	<< "	<kerning first=\"87\" second=\"121\" amount=\"-1\" />																																			"
	<< "	<kerning first=\"89\" second=\"32\" amount=\"-1\" />																																			"
	<< "	<kerning first=\"89\" second=\"44\" amount=\"-8\" />																																			"
	<< "	<kerning first=\"89\" second=\"45\" amount=\"-6\" />																																			"
	<< "	<kerning first=\"89\" second=\"46\" amount=\"-8\" />																																			"
	<< "	<kerning first=\"89\" second=\"58\" amount=\"-4\" />																																			"
	<< "	<kerning first=\"89\" second=\"111\" amount=\"-6\" />																																			"
	<< "	<kerning first=\"89\" second=\"65\" amount=\"-5\" />																																			"
	<< "	<kerning first=\"89\" second=\"97\" amount=\"-5\" />																																			"
	<< "	<kerning first=\"89\" second=\"101\" amount=\"-6\" />																																			"
	<< "	<kerning first=\"89\" second=\"105\" amount=\"-2\" />																																			"
	<< "	</kernings>																																														"
	<< "	</font>";
#pragma endregion Arial_Normal_Xml

#pragma region BitFont_Xml
	m_buildInFontXML[BITFONT]
	<< "	<?xml version=\"1.0\"?>																																												"
	<< "	<font>																																																"
	<< "	<info face=\"Fixedsys\" size=\"90\" bold=\"0\" italic=\"0\" charset=\"\" unicode=\"1\" stretchH=\"100\" smooth=\"1\" aa=\"1\" padding=\"3,3,3,3\" spacing=\"1,1\" outline=\"0\"/>					"
	<< "	<common lineHeight=\"90\" base=\"72\" scaleW=\"1024\" scaleH=\"1024\" pages=\"1\" packed=\"0\" alphaChnl=\"0\" redChnl=\"4\" greenChnl=\"4\" blueChnl=\"4\"/>										"
	<< "	<pages>																																																"
	<< "	<page id=\"0\" file=\"bitFont_0.png\" />																																							"
	<< "	</pages>																																															"
	<< "	<chars count=\"205\">																																												"
	<< "	<char id=\"32\" x=\"0\" y=\"0\" width=\"126\" height=\"96\" xoffset=\"-43\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"33\" x=\"707\" y=\"679\" width=\"26\" height=\"96\" xoffset=\"7\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"34\" x=\"185\" y=\"582\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"35\" x=\"747\" y=\"97\" width=\"41\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"36\" x=\"592\" y=\"582\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"37\" x=\"94\" y=\"97\" width=\"46\" height=\"96\" xoffset=\"-3\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"38\" x=\"789\" y=\"97\" width=\"41\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"39\" x=\"972\" y=\"679\" width=\"16\" height=\"96\" xoffset=\"12\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"40\" x=\"923\" y=\"679\" width=\"26\" height=\"96\" xoffset=\"7\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"41\" x=\"896\" y=\"679\" width=\"26\" height=\"96\" xoffset=\"7\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"42\" x=\"168\" y=\"194\" width=\"41\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"43\" x=\"740\" y=\"582\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"44\" x=\"999\" y=\"582\" width=\"21\" height=\"96\" xoffset=\"12\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"45\" x=\"37\" y=\"679\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"46\" x=\"999\" y=\"485\" width=\"21\" height=\"96\" xoffset=\"12\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"47\" x=\"111\" y=\"679\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"48\" x=\"983\" y=\"0\" width=\"36\" height=\"96\" xoffset=\"7\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																	"
	<< "	<char id=\"49\" x=\"444\" y=\"679\" width=\"31\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"50\" x=\"504\" y=\"194\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"51\" x=\"541\" y=\"194\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"52\" x=\"42\" y=\"194\" width=\"41\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"53\" x=\"578\" y=\"194\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"54\" x=\"615\" y=\"194\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"55\" x=\"652\" y=\"194\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"56\" x=\"689\" y=\"194\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"57\" x=\"726\" y=\"194\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"58\" x=\"999\" y=\"388\" width=\"21\" height=\"96\" xoffset=\"12\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"59\" x=\"999\" y=\"291\" width=\"21\" height=\"96\" xoffset=\"12\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"60\" x=\"763\" y=\"194\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"61\" x=\"800\" y=\"194\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"62\" x=\"837\" y=\"194\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"63\" x=\"874\" y=\"194\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"64\" x=\"0\" y=\"97\" width=\"46\" height=\"96\" xoffset=\"-3\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																	"
	<< "	<char id=\"65\" x=\"911\" y=\"194\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"66\" x=\"948\" y=\"194\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"67\" x=\"985\" y=\"194\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"68\" x=\"0\" y=\"291\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																	"
	<< "	<char id=\"69\" x=\"37\" y=\"291\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"70\" x=\"74\" y=\"291\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"71\" x=\"111\" y=\"291\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"72\" x=\"148\" y=\"291\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"73\" x=\"599\" y=\"679\" width=\"26\" height=\"96\" xoffset=\"7\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"74\" x=\"185\" y=\"291\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"75\" x=\"222\" y=\"291\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"76\" x=\"259\" y=\"291\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"77\" x=\"336\" y=\"194\" width=\"41\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"78\" x=\"126\" y=\"194\" width=\"41\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"79\" x=\"296\" y=\"291\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"80\" x=\"333\" y=\"291\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"81\" x=\"370\" y=\"291\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"82\" x=\"407\" y=\"291\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"83\" x=\"444\" y=\"291\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"84\" x=\"481\" y=\"291\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"85\" x=\"518\" y=\"291\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"86\" x=\"555\" y=\"291\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"87\" x=\"84\" y=\"194\" width=\"41\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"88\" x=\"592\" y=\"291\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"89\" x=\"629\" y=\"291\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"90\" x=\"666\" y=\"291\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"91\" x=\"788\" y=\"679\" width=\"26\" height=\"96\" xoffset=\"7\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"92\" x=\"703\" y=\"291\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"93\" x=\"761\" y=\"679\" width=\"26\" height=\"96\" xoffset=\"7\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"94\" x=\"740\" y=\"291\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"95\" x=\"376\" y=\"97\" width=\"46\" height=\"96\" xoffset=\"-3\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"96\" x=\"626\" y=\"679\" width=\"26\" height=\"96\" xoffset=\"7\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"97\" x=\"777\" y=\"291\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"98\" x=\"814\" y=\"291\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"99\" x=\"851\" y=\"291\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"100\" x=\"888\" y=\"291\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"101\" x=\"925\" y=\"291\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"102\" x=\"962\" y=\"291\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"103\" x=\"0\" y=\"388\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"104\" x=\"37\" y=\"388\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"105\" x=\"74\" y=\"388\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"106\" x=\"476\" y=\"679\" width=\"31\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"107\" x=\"111\" y=\"388\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"108\" x=\"148\" y=\"388\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"109\" x=\"705\" y=\"97\" width=\"41\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"110\" x=\"185\" y=\"388\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"111\" x=\"222\" y=\"388\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"112\" x=\"259\" y=\"388\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"113\" x=\"296\" y=\"388\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"114\" x=\"333\" y=\"388\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"115\" x=\"370\" y=\"388\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"116\" x=\"407\" y=\"388\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"117\" x=\"444\" y=\"388\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"118\" x=\"481\" y=\"388\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"119\" x=\"0\" y=\"194\" width=\"41\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"120\" x=\"518\" y=\"388\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"121\" x=\"210\" y=\"194\" width=\"41\" height=\"96\" xoffset=\"-3\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"122\" x=\"555\" y=\"388\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"123\" x=\"540\" y=\"679\" width=\"31\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"124\" x=\"1006\" y=\"679\" width=\"16\" height=\"96\" xoffset=\"12\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"125\" x=\"508\" y=\"679\" width=\"31\" height=\"96\" xoffset=\"7\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"126\" x=\"658\" y=\"97\" width=\"46\" height=\"96\" xoffset=\"-3\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"127\" x=\"592\" y=\"388\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"129\" x=\"381\" y=\"0\" width=\"126\" height=\"96\" xoffset=\"-43\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"141\" x=\"254\" y=\"0\" width=\"126\" height=\"96\" xoffset=\"-43\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"143\" x=\"762\" y=\"0\" width=\"126\" height=\"96\" xoffset=\"-43\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"144\" x=\"508\" y=\"0\" width=\"126\" height=\"96\" xoffset=\"-43\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"157\" x=\"635\" y=\"0\" width=\"126\" height=\"96\" xoffset=\"-43\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"160\" x=\"127\" y=\"0\" width=\"126\" height=\"96\" xoffset=\"-43\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"161\" x=\"572\" y=\"679\" width=\"26\" height=\"96\" xoffset=\"7\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"162\" x=\"666\" y=\"388\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"163\" x=\"294\" y=\"194\" width=\"41\" height=\"96\" xoffset=\"-3\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"164\" x=\"703\" y=\"388\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"165\" x=\"740\" y=\"388\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"166\" x=\"989\" y=\"679\" width=\"16\" height=\"96\" xoffset=\"12\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"167\" x=\"777\" y=\"388\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"168\" x=\"814\" y=\"388\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"169\" x=\"611\" y=\"97\" width=\"46\" height=\"96\" xoffset=\"-3\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"170\" x=\"851\" y=\"388\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"171\" x=\"517\" y=\"97\" width=\"46\" height=\"96\" xoffset=\"-3\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"172\" x=\"888\" y=\"388\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"173\" x=\"925\" y=\"388\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"174\" x=\"141\" y=\"97\" width=\"46\" height=\"96\" xoffset=\"-3\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"175\" x=\"423\" y=\"97\" width=\"46\" height=\"96\" xoffset=\"-3\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"176\" x=\"962\" y=\"388\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"177\" x=\"0\" y=\"485\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"178\" x=\"653\" y=\"679\" width=\"26\" height=\"96\" xoffset=\"7\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"179\" x=\"680\" y=\"679\" width=\"26\" height=\"96\" xoffset=\"7\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"180\" x=\"734\" y=\"679\" width=\"26\" height=\"96\" xoffset=\"7\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"181\" x=\"47\" y=\"97\" width=\"46\" height=\"96\" xoffset=\"-3\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"182\" x=\"252\" y=\"194\" width=\"41\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"183\" x=\"950\" y=\"679\" width=\"21\" height=\"96\" xoffset=\"12\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"184\" x=\"869\" y=\"679\" width=\"26\" height=\"96\" xoffset=\"7\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"185\" x=\"999\" y=\"97\" width=\"21\" height=\"96\" xoffset=\"7\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"186\" x=\"629\" y=\"388\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"187\" x=\"329\" y=\"97\" width=\"46\" height=\"96\" xoffset=\"-3\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"188\" x=\"936\" y=\"0\" width=\"46\" height=\"96\" xoffset=\"-3\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"189\" x=\"564\" y=\"97\" width=\"46\" height=\"96\" xoffset=\"-3\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"190\" x=\"470\" y=\"97\" width=\"46\" height=\"96\" xoffset=\"-3\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"191\" x=\"37\" y=\"485\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"192\" x=\"74\" y=\"485\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"193\" x=\"111\" y=\"485\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"194\" x=\"148\" y=\"485\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"195\" x=\"378\" y=\"194\" width=\"41\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"196\" x=\"185\" y=\"485\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"197\" x=\"222\" y=\"485\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"198\" x=\"957\" y=\"97\" width=\"41\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"199\" x=\"259\" y=\"485\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"200\" x=\"296\" y=\"485\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"201\" x=\"333\" y=\"485\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"202\" x=\"370\" y=\"485\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"203\" x=\"407\" y=\"485\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"204\" x=\"815\" y=\"679\" width=\"26\" height=\"96\" xoffset=\"7\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"205\" x=\"842\" y=\"679\" width=\"26\" height=\"96\" xoffset=\"7\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"206\" x=\"444\" y=\"485\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"207\" x=\"481\" y=\"485\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"208\" x=\"915\" y=\"97\" width=\"41\" height=\"96\" xoffset=\"-3\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"209\" x=\"873\" y=\"97\" width=\"41\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"210\" x=\"518\" y=\"485\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"211\" x=\"555\" y=\"485\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"212\" x=\"592\" y=\"485\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"213\" x=\"831\" y=\"97\" width=\"41\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"214\" x=\"629\" y=\"485\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"215\" x=\"666\" y=\"485\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"216\" x=\"703\" y=\"485\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"217\" x=\"740\" y=\"485\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"218\" x=\"777\" y=\"485\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"219\" x=\"814\" y=\"485\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"220\" x=\"851\" y=\"485\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"221\" x=\"888\" y=\"485\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"222\" x=\"925\" y=\"485\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"223\" x=\"962\" y=\"485\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"224\" x=\"0\" y=\"582\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"225\" x=\"37\" y=\"582\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"226\" x=\"74\" y=\"582\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"227\" x=\"889\" y=\"0\" width=\"46\" height=\"96\" xoffset=\"-3\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"228\" x=\"111\" y=\"582\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"229\" x=\"148\" y=\"582\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"230\" x=\"188\" y=\"97\" width=\"46\" height=\"96\" xoffset=\"-3\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"231\" x=\"222\" y=\"582\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"232\" x=\"259\" y=\"582\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"233\" x=\"296\" y=\"582\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"234\" x=\"333\" y=\"582\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"235\" x=\"370\" y=\"582\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"236\" x=\"407\" y=\"582\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"237\" x=\"444\" y=\"582\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"238\" x=\"481\" y=\"582\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"239\" x=\"518\" y=\"582\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"240\" x=\"555\" y=\"582\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"241\" x=\"235\" y=\"97\" width=\"46\" height=\"96\" xoffset=\"-3\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"242\" x=\"629\" y=\"582\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"243\" x=\"666\" y=\"582\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"244\" x=\"703\" y=\"582\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"245\" x=\"282\" y=\"97\" width=\"46\" height=\"96\" xoffset=\"-3\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"246\" x=\"777\" y=\"582\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"247\" x=\"814\" y=\"582\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"248\" x=\"851\" y=\"582\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"249\" x=\"888\" y=\"582\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"250\" x=\"925\" y=\"582\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"251\" x=\"962\" y=\"582\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"252\" x=\"0\" y=\"679\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"253\" x=\"420\" y=\"194\" width=\"41\" height=\"96\" xoffset=\"-3\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"254\" x=\"74\" y=\"679\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"255\" x=\"462\" y=\"194\" width=\"41\" height=\"96\" xoffset=\"-3\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"338\" x=\"148\" y=\"679\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"339\" x=\"185\" y=\"679\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"352\" x=\"222\" y=\"679\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"353\" x=\"259\" y=\"679\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"376\" x=\"296\" y=\"679\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"381\" x=\"333\" y=\"679\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"382\" x=\"370\" y=\"679\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"402\" x=\"407\" y=\"679\" width=\"36\" height=\"96\" xoffset=\"2\" yoffset=\"-3\" xadvance=\"40\" page=\"0\" chnl=\"15\" />																"
	<< "	</chars>																																															"
	<< "	</font>";
#pragma endregion BitFont_Xml

#pragma region BookAntiqua_Xml
	m_buildInFontXML[BOOKANTIQUA]
	<< "	<?xml version=\"1.0\"?>																																											"
	<< "	<font>																																															"
	<< "	<info face=\"Book Antiqua\" size=\"64\" bold=\"1\" italic=\"0\" charset=\"\" unicode=\"1\" stretchH=\"100\" smooth=\"1\" aa=\"1\" padding=\"0,0,0,0\" spacing=\"1,1\" outline=\"1\"/>			"
	<< "	<common lineHeight=\"63\" base=\"50\" scaleW=\"1024\" scaleH=\"1024\" pages=\"1\" packed=\"0\" alphaChnl=\"0\" redChnl=\"4\" greenChnl=\"4\" blueChnl=\"4\"/>									"
	<< "	<pages>																																															"
	<< "	<page id=\"0\" file=\"BookAntiqua_0.png\" />																																					"
	<< "	</pages>																																														"
	<< "	<chars count=\"325\">																																											"
	<< "	<char id=\"32\" x=\"1016\" y=\"396\" width=\"5\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"13\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"33\" x=\"155\" y=\"660\" width=\"12\" height=\"65\" xoffset=\"2\" yoffset=\"-1\" xadvance=\"15\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"34\" x=\"945\" y=\"594\" width=\"18\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"35\" x=\"644\" y=\"396\" width=\"30\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"36\" x=\"196\" y=\"528\" width=\"27\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"37\" x=\"433\" y=\"0\" width=\"47\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"47\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"38\" x=\"899\" y=\"66\" width=\"43\" height=\"65\" xoffset=\"2\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"39\" x=\"204\" y=\"660\" width=\"10\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"12\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"40\" x=\"71\" y=\"660\" width=\"16\" height=\"65\" xoffset=\"2\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"41\" x=\"1001\" y=\"594\" width=\"17\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"42\" x=\"215\" y=\"594\" width=\"22\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"24\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"43\" x=\"830\" y=\"396\" width=\"30\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"44\" x=\"139\" y=\"660\" width=\"15\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"13\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"45\" x=\"192\" y=\"594\" width=\"22\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"46\" x=\"192\" y=\"660\" width=\"11\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"13\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"47\" x=\"571\" y=\"462\" width=\"28\" height=\"65\" xoffset=\"-8\" yoffset=\"-1\" xadvance=\"16\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"48\" x=\"140\" y=\"528\" width=\"27\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"49\" x=\"84\" y=\"528\" width=\"27\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"50\" x=\"452\" y=\"462\" width=\"29\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"51\" x=\"28\" y=\"528\" width=\"27\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"52\" x=\"332\" y=\"462\" width=\"29\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"53\" x=\"252\" y=\"528\" width=\"27\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"54\" x=\"308\" y=\"528\" width=\"27\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"55\" x=\"62\" y=\"462\" width=\"29\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"56\" x=\"420\" y=\"528\" width=\"27\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"57\" x=\"0\" y=\"528\" width=\"27\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"58\" x=\"180\" y=\"660\" width=\"11\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"13\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"59\" x=\"88\" y=\"660\" width=\"16\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"13\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"60\" x=\"737\" y=\"396\" width=\"30\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"61\" x=\"892\" y=\"396\" width=\"30\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"62\" x=\"675\" y=\"396\" width=\"30\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"63\" x=\"816\" y=\"528\" width=\"24\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"24\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"64\" x=\"423\" y=\"198\" width=\"40\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"40\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"65\" x=\"86\" y=\"198\" width=\"42\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"66\" x=\"378\" y=\"264\" width=\"34\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"35\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"67\" x=\"901\" y=\"198\" width=\"38\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"38\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"68\" x=\"630\" y=\"66\" width=\"44\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"69\" x=\"339\" y=\"330\" width=\"32\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"70\" x=\"923\" y=\"396\" width=\"30\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"29\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"71\" x=\"43\" y=\"132\" width=\"42\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"72\" x=\"0\" y=\"66\" width=\"44\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"73\" x=\"351\" y=\"594\" width=\"21\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"74\" x=\"941\" y=\"528\" width=\"23\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"75\" x=\"129\" y=\"132\" width=\"42\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"76\" x=\"405\" y=\"330\" width=\"32\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"77\" x=\"228\" y=\"0\" width=\"53\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"53\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"78\" x=\"540\" y=\"66\" width=\"44\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"79\" x=\"675\" y=\"66\" width=\"44\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"80\" x=\"652\" y=\"264\" width=\"33\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"81\" x=\"719\" y=\"0\" width=\"45\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"82\" x=\"666\" y=\"198\" width=\"39\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"38\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"83\" x=\"985\" y=\"396\" width=\"30\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"84\" x=\"986\" y=\"66\" width=\"37\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"35\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"85\" x=\"215\" y=\"132\" width=\"42\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"86\" x=\"258\" y=\"132\" width=\"42\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"87\" x=\"0\" y=\"0\" width=\"59\" height=\"65\" xoffset=\"-3\" yoffset=\"-1\" xadvance=\"53\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"88\" x=\"78\" y=\"264\" width=\"37\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"35\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"89\" x=\"862\" y=\"198\" width=\"38\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"35\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"90\" x=\"341\" y=\"264\" width=\"36\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"35\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"91\" x=\"0\" y=\"660\" width=\"17\" height=\"65\" xoffset=\"2\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />																"
	<< "	<char id=\"92\" x=\"550\" y=\"396\" width=\"31\" height=\"65\" xoffset=\"2\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"93\" x=\"18\" y=\"660\" width=\"17\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"94\" x=\"31\" y=\"462\" width=\"30\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"95\" x=\"992\" y=\"264\" width=\"31\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"96\" x=\"105\" y=\"660\" width=\"16\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"97\" x=\"774\" y=\"462\" width=\"28\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"98\" x=\"768\" y=\"330\" width=\"32\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"99\" x=\"791\" y=\"528\" width=\"24\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"24\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"100\" x=\"702\" y=\"330\" width=\"32\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"101\" x=\"56\" y=\"528\" width=\"27\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"102\" x=\"989\" y=\"528\" width=\"23\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"103\" x=\"198\" y=\"396\" width=\"31\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"29\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"104\" x=\"788\" y=\"264\" width=\"33\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"105\" x=\"774\" y=\"594\" width=\"18\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"106\" x=\"983\" y=\"594\" width=\"17\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"107\" x=\"238\" y=\"330\" width=\"33\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"108\" x=\"869\" y=\"594\" width=\"18\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"109\" x=\"336\" y=\"0\" width=\"48\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"47\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"110\" x=\"822\" y=\"264\" width=\"33\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"111\" x=\"422\" y=\"462\" width=\"29\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"29\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"112\" x=\"326\" y=\"396\" width=\"31\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"113\" x=\"230\" y=\"396\" width=\"31\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"114\" x=\"965\" y=\"528\" width=\"23\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"115\" x=\"741\" y=\"528\" width=\"24\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"24\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"116\" x=\"570\" y=\"594\" width=\"20\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"117\" x=\"754\" y=\"264\" width=\"33\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"118\" x=\"33\" y=\"396\" width=\"32\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"29\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"119\" x=\"481\" y=\"0\" width=\"47\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"120\" x=\"122\" y=\"462\" width=\"29\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"121\" x=\"966\" y=\"330\" width=\"32\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"29\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"122\" x=\"629\" y=\"462\" width=\"28\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"123\" x=\"36\" y=\"660\" width=\"17\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"16\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"124\" x=\"224\" y=\"660\" width=\"8\" height=\"65\" xoffset=\"12\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"125\" x=\"1006\" y=\"462\" width=\"17\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"16\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"126\" x=\"66\" y=\"396\" width=\"32\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"160\" x=\"1018\" y=\"198\" width=\"5\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"13\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"161\" x=\"168\" y=\"660\" width=\"11\" height=\"65\" xoffset=\"2\" yoffset=\"-1\" xadvance=\"15\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"162\" x=\"144\" y=\"594\" width=\"23\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"163\" x=\"392\" y=\"462\" width=\"29\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"164\" x=\"799\" y=\"396\" width=\"30\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"165\" x=\"989\" y=\"132\" width=\"34\" height=\"65\" xoffset=\"-5\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"166\" x=\"215\" y=\"660\" width=\"8\" height=\"65\" xoffset=\"12\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"167\" x=\"476\" y=\"528\" width=\"27\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"168\" x=\"549\" y=\"594\" width=\"20\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"169\" x=\"341\" y=\"198\" width=\"40\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"40\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"170\" x=\"674\" y=\"594\" width=\"19\" height=\"65\" xoffset=\"2\" yoffset=\"-1\" xadvance=\"23\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"171\" x=\"587\" y=\"528\" width=\"26\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"172\" x=\"582\" y=\"396\" width=\"30\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"173\" x=\"284\" y=\"594\" width=\"22\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"174\" x=\"464\" y=\"198\" width=\"40\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"40\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"175\" x=\"262\" y=\"396\" width=\"31\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"176\" x=\"694\" y=\"594\" width=\"19\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"177\" x=\"362\" y=\"462\" width=\"29\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"29\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"178\" x=\"714\" y=\"594\" width=\"19\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"19\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"179\" x=\"734\" y=\"594\" width=\"19\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"19\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"180\" x=\"122\" y=\"660\" width=\"16\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"181\" x=\"669\" y=\"330\" width=\"32\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"31\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"182\" x=\"413\" y=\"264\" width=\"34\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"34\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"183\" x=\"1013\" y=\"528\" width=\"10\" height=\"65\" xoffset=\"13\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />														"
	<< "	<char id=\"184\" x=\"54\" y=\"660\" width=\"16\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"185\" x=\"850\" y=\"594\" width=\"18\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"19\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"186\" x=\"591\" y=\"594\" width=\"20\" height=\"65\" xoffset=\"3\" yoffset=\"-1\" xadvance=\"26\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"187\" x=\"614\" y=\"528\" width=\"26\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"188\" x=\"625\" y=\"0\" width=\"47\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"47\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"189\" x=\"577\" y=\"0\" width=\"47\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"47\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"190\" x=\"529\" y=\"0\" width=\"47\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"47\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"191\" x=\"766\" y=\"528\" width=\"24\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"24\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"192\" x=\"860\" y=\"132\" width=\"42\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"193\" x=\"903\" y=\"132\" width=\"42\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"194\" x=\"946\" y=\"132\" width=\"42\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"195\" x=\"0\" y=\"198\" width=\"42\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"196\" x=\"43\" y=\"198\" width=\"42\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"197\" x=\"129\" y=\"198\" width=\"42\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"198\" x=\"282\" y=\"0\" width=\"53\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"53\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"199\" x=\"940\" y=\"198\" width=\"38\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"38\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"200\" x=\"471\" y=\"330\" width=\"32\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"201\" x=\"834\" y=\"330\" width=\"32\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"202\" x=\"165\" y=\"396\" width=\"32\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"203\" x=\"570\" y=\"330\" width=\"32\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"204\" x=\"461\" y=\"594\" width=\"21\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"205\" x=\"483\" y=\"594\" width=\"21\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"206\" x=\"439\" y=\"594\" width=\"21\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"207\" x=\"527\" y=\"594\" width=\"21\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"208\" x=\"315\" y=\"66\" width=\"44\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"209\" x=\"225\" y=\"66\" width=\"44\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"210\" x=\"135\" y=\"66\" width=\"44\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"211\" x=\"90\" y=\"66\" width=\"44\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"212\" x=\"45\" y=\"66\" width=\"44\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"213\" x=\"901\" y=\"0\" width=\"44\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"214\" x=\"811\" y=\"0\" width=\"44\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"216\" x=\"673\" y=\"0\" width=\"45\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"217\" x=\"172\" y=\"198\" width=\"42\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"218\" x=\"215\" y=\"198\" width=\"42\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"219\" x=\"86\" y=\"132\" width=\"42\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"220\" x=\"172\" y=\"132\" width=\"42\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"221\" x=\"0\" y=\"264\" width=\"38\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"35\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"222\" x=\"170\" y=\"330\" width=\"33\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"223\" x=\"294\" y=\"396\" width=\"31\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"224\" x=\"832\" y=\"462\" width=\"28\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"225\" x=\"803\" y=\"462\" width=\"28\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"226\" x=\"745\" y=\"462\" width=\"28\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"227\" x=\"658\" y=\"462\" width=\"28\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"228\" x=\"687\" y=\"462\" width=\"28\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"229\" x=\"919\" y=\"462\" width=\"28\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"230\" x=\"382\" y=\"198\" width=\"40\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"231\" x=\"716\" y=\"528\" width=\"24\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"24\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"232\" x=\"112\" y=\"528\" width=\"27\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"233\" x=\"168\" y=\"528\" width=\"27\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"234\" x=\"224\" y=\"528\" width=\"27\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"235\" x=\"280\" y=\"528\" width=\"27\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"236\" x=\"907\" y=\"594\" width=\"18\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"237\" x=\"964\" y=\"594\" width=\"18\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"238\" x=\"633\" y=\"594\" width=\"20\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"239\" x=\"612\" y=\"594\" width=\"20\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"240\" x=\"92\" y=\"462\" width=\"29\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"29\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"241\" x=\"0\" y=\"330\" width=\"33\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"242\" x=\"152\" y=\"462\" width=\"29\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"29\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"243\" x=\"182\" y=\"462\" width=\"29\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"29\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"244\" x=\"212\" y=\"462\" width=\"29\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"29\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"245\" x=\"242\" y=\"462\" width=\"29\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"29\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"246\" x=\"272\" y=\"462\" width=\"29\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"29\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"247\" x=\"302\" y=\"462\" width=\"29\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"29\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"248\" x=\"933\" y=\"330\" width=\"32\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"29\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"249\" x=\"890\" y=\"264\" width=\"33\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"250\" x=\"136\" y=\"330\" width=\"33\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"251\" x=\"102\" y=\"330\" width=\"33\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"252\" x=\"68\" y=\"330\" width=\"33\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"253\" x=\"603\" y=\"330\" width=\"32\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"29\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"254\" x=\"358\" y=\"396\" width=\"31\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"255\" x=\"0\" y=\"396\" width=\"32\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"29\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"256\" x=\"817\" y=\"132\" width=\"42\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"257\" x=\"977\" y=\"462\" width=\"28\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"258\" x=\"774\" y=\"132\" width=\"42\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"259\" x=\"542\" y=\"462\" width=\"28\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"260\" x=\"688\" y=\"132\" width=\"42\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"261\" x=\"600\" y=\"462\" width=\"28\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"262\" x=\"823\" y=\"198\" width=\"38\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"38\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"263\" x=\"999\" y=\"330\" width=\"24\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"24\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"264\" x=\"39\" y=\"264\" width=\"38\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"38\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"265\" x=\"641\" y=\"528\" width=\"24\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"24\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"266\" x=\"784\" y=\"198\" width=\"38\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"38\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"267\" x=\"666\" y=\"528\" width=\"24\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"24\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"268\" x=\"979\" y=\"198\" width=\"38\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"38\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"269\" x=\"691\" y=\"528\" width=\"24\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"24\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"270\" x=\"720\" y=\"66\" width=\"44\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"271\" x=\"505\" y=\"198\" width=\"40\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"39\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"272\" x=\"765\" y=\"66\" width=\"44\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"273\" x=\"438\" y=\"330\" width=\"32\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"274\" x=\"504\" y=\"330\" width=\"32\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"275\" x=\"336\" y=\"528\" width=\"27\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"276\" x=\"537\" y=\"330\" width=\"32\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"277\" x=\"364\" y=\"528\" width=\"27\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"278\" x=\"735\" y=\"330\" width=\"32\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"279\" x=\"392\" y=\"528\" width=\"27\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"280\" x=\"801\" y=\"330\" width=\"32\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"281\" x=\"448\" y=\"528\" width=\"27\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"282\" x=\"867\" y=\"330\" width=\"32\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"283\" x=\"504\" y=\"528\" width=\"27\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"284\" x=\"602\" y=\"132\" width=\"42\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"285\" x=\"390\" y=\"396\" width=\"31\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"29\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"286\" x=\"516\" y=\"132\" width=\"42\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"287\" x=\"422\" y=\"396\" width=\"31\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"29\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"288\" x=\"473\" y=\"132\" width=\"42\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"289\" x=\"454\" y=\"396\" width=\"31\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"29\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"290\" x=\"301\" y=\"132\" width=\"42\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"291\" x=\"486\" y=\"396\" width=\"31\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"29\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"292\" x=\"450\" y=\"66\" width=\"44\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"293\" x=\"272\" y=\"330\" width=\"33\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"294\" x=\"495\" y=\"66\" width=\"44\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"295\" x=\"204\" y=\"330\" width=\"33\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"296\" x=\"24\" y=\"594\" width=\"23\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"297\" x=\"72\" y=\"594\" width=\"23\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"298\" x=\"395\" y=\"594\" width=\"21\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"299\" x=\"373\" y=\"594\" width=\"21\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"300\" x=\"329\" y=\"594\" width=\"21\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"301\" x=\"654\" y=\"594\" width=\"19\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"302\" x=\"307\" y=\"594\" width=\"21\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"303\" x=\"831\" y=\"594\" width=\"18\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"304\" x=\"505\" y=\"594\" width=\"21\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"305\" x=\"812\" y=\"594\" width=\"18\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"306\" x=\"258\" y=\"198\" width=\"41\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"307\" x=\"958\" y=\"264\" width=\"33\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"35\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"308\" x=\"0\" y=\"594\" width=\"23\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"309\" x=\"754\" y=\"594\" width=\"19\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"310\" x=\"387\" y=\"132\" width=\"42\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"311\" x=\"720\" y=\"264\" width=\"33\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"312\" x=\"686\" y=\"264\" width=\"33\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"313\" x=\"900\" y=\"330\" width=\"32\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"314\" x=\"793\" y=\"594\" width=\"18\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"315\" x=\"132\" y=\"396\" width=\"32\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"316\" x=\"888\" y=\"594\" width=\"18\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"317\" x=\"991\" y=\"0\" width=\"32\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"318\" x=\"560\" y=\"528\" width=\"26\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"24\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"319\" x=\"99\" y=\"396\" width=\"32\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"320\" x=\"238\" y=\"594\" width=\"22\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"321\" x=\"636\" y=\"330\" width=\"32\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"322\" x=\"926\" y=\"594\" width=\"18\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"323\" x=\"810\" y=\"66\" width=\"44\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"324\" x=\"924\" y=\"264\" width=\"33\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"325\" x=\"585\" y=\"66\" width=\"44\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"326\" x=\"856\" y=\"264\" width=\"33\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"327\" x=\"405\" y=\"66\" width=\"44\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"328\" x=\"34\" y=\"330\" width=\"33\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"329\" x=\"270\" y=\"66\" width=\"44\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"330\" x=\"855\" y=\"66\" width=\"43\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"43\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"331\" x=\"0\" y=\"462\" width=\"30\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"33\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"332\" x=\"180\" y=\"66\" width=\"44\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"333\" x=\"512\" y=\"462\" width=\"29\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"29\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"334\" x=\"856\" y=\"0\" width=\"44\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"335\" x=\"482\" y=\"462\" width=\"29\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"29\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"336\" x=\"360\" y=\"66\" width=\"44\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"337\" x=\"954\" y=\"396\" width=\"30\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"29\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"338\" x=\"120\" y=\"0\" width=\"53\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"53\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"339\" x=\"946\" y=\"0\" width=\"44\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"340\" x=\"626\" y=\"198\" width=\"39\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"38\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"341\" x=\"96\" y=\"594\" width=\"23\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"342\" x=\"586\" y=\"198\" width=\"39\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"38\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"343\" x=\"120\" y=\"594\" width=\"23\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"344\" x=\"546\" y=\"198\" width=\"39\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"38\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"345\" x=\"48\" y=\"594\" width=\"23\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"21\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"346\" x=\"861\" y=\"396\" width=\"30\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"347\" x=\"841\" y=\"528\" width=\"24\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"24\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"348\" x=\"613\" y=\"396\" width=\"30\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"349\" x=\"866\" y=\"528\" width=\"24\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"24\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"350\" x=\"768\" y=\"396\" width=\"30\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"351\" x=\"891\" y=\"528\" width=\"24\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"24\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"352\" x=\"706\" y=\"396\" width=\"30\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"353\" x=\"916\" y=\"528\" width=\"24\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"24\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"354\" x=\"192\" y=\"264\" width=\"37\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"35\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"355\" x=\"417\" y=\"594\" width=\"21\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"356\" x=\"154\" y=\"264\" width=\"37\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"35\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"357\" x=\"532\" y=\"528\" width=\"27\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"22\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"358\" x=\"116\" y=\"264\" width=\"37\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"35\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"359\" x=\"261\" y=\"594\" width=\"22\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"360\" x=\"731\" y=\"132\" width=\"42\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"361\" x=\"618\" y=\"264\" width=\"33\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"362\" x=\"645\" y=\"132\" width=\"42\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"363\" x=\"584\" y=\"264\" width=\"33\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"364\" x=\"559\" y=\"132\" width=\"42\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"365\" x=\"448\" y=\"264\" width=\"33\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"366\" x=\"943\" y=\"66\" width=\"42\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"367\" x=\"550\" y=\"264\" width=\"33\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"368\" x=\"430\" y=\"132\" width=\"42\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"369\" x=\"516\" y=\"264\" width=\"33\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"370\" x=\"344\" y=\"132\" width=\"42\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"371\" x=\"482\" y=\"264\" width=\"33\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"32\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"372\" x=\"60\" y=\"0\" width=\"59\" height=\"65\" xoffset=\"-3\" yoffset=\"-1\" xadvance=\"53\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"373\" x=\"385\" y=\"0\" width=\"47\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"374\" x=\"745\" y=\"198\" width=\"38\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"35\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"375\" x=\"306\" y=\"330\" width=\"32\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"29\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"376\" x=\"706\" y=\"198\" width=\"38\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"35\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"377\" x=\"304\" y=\"264\" width=\"36\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"35\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"378\" x=\"948\" y=\"462\" width=\"28\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"379\" x=\"267\" y=\"264\" width=\"36\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"35\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"380\" x=\"890\" y=\"462\" width=\"28\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"381\" x=\"230\" y=\"264\" width=\"36\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"35\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"382\" x=\"861\" y=\"462\" width=\"28\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"383\" x=\"168\" y=\"594\" width=\"23\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"18\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"402\" x=\"518\" y=\"396\" width=\"31\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"506\" x=\"0\" y=\"132\" width=\"42\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"507\" x=\"716\" y=\"462\" width=\"28\" height=\"65\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"27\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"508\" x=\"174\" y=\"0\" width=\"53\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"53\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"509\" x=\"300\" y=\"198\" width=\"40\" height=\"65\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"41\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"510\" x=\"765\" y=\"0\" width=\"45\" height=\"65\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"44\" page=\"0\" chnl=\"15\" />															"
	<< "	<char id=\"511\" x=\"372\" y=\"330\" width=\"32\" height=\"65\" xoffset=\"-2\" yoffset=\"-1\" xadvance=\"29\" page=\"0\" chnl=\"15\" />															"
	<< "	</chars>																																														"
	<< "	</font>";
#pragma endregion BookAntiqua_Xml

#pragma region Buxton_Xml
	m_buildInFontXML[BUXTON]
	<< "	<?xml version=\"1.0\"?>																																									"
	<< "	<font>																																													"
	<< "	<info face=\"Buxton Sketch\" size=\"-60\" bold=\"0\" italic=\"0\" charset=\"\" unicode=\"1\" stretchH=\"100\" smooth=\"1\" aa=\"1\" padding=\"0,0,0,0\" spacing=\"1,1\" outline=\"5\"/>	"
	<< "	<common lineHeight=\"74\" base=\"51\" scaleW=\"1024\" scaleH=\"1024\" pages=\"1\" packed=\"0\" alphaChnl=\"1\" redChnl=\"0\" greenChnl=\"0\" blueChnl=\"0\"/>							"
	<< "	<pages>																																													"
	<< "	<page id=\"0\" file=\"buxton_0.png\" />																																					"
	<< "	</pages>																																												"
	<< "	<chars count=\"331\">																																									"
	<< "	<char id=\"32\" x=\"1010\" y=\"680\" width=\"13\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"14\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"33\" x=\"277\" y=\"935\" width=\"24\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"15\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"34\" x=\"58\" y=\"935\" width=\"27\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"19\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"35\" x=\"230\" y=\"0\" width=\"51\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"38\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"36\" x=\"351\" y=\"340\" width=\"38\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"37\" x=\"396\" y=\"170\" width=\"42\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"33\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"38\" x=\"96\" y=\"85\" width=\"47\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"39\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"39\" x=\"1006\" y=\"255\" width=\"17\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"9\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"40\" x=\"544\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-1\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"41\" x=\"888\" y=\"510\" width=\"35\" height=\"84\" xoffset=\"-12\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"42\" x=\"967\" y=\"85\" width=\"43\" height=\"84\" xoffset=\"-7\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"43\" x=\"140\" y=\"680\" width=\"34\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"44\" x=\"1003\" y=\"595\" width=\"20\" height=\"84\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"15\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"45\" x=\"979\" y=\"850\" width=\"28\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"46\" x=\"814\" y=\"935\" width=\"19\" height=\"84\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"15\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"47\" x=\"688\" y=\"850\" width=\"29\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"16\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"48\" x=\"936\" y=\"340\" width=\"38\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"49\" x=\"712\" y=\"935\" width=\"20\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"15\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"50\" x=\"740\" y=\"170\" width=\"41\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"51\" x=\"88\" y=\"170\" width=\"43\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"35\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"52\" x=\"824\" y=\"170\" width=\"41\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"53\" x=\"841\" y=\"425\" width=\"36\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"54\" x=\"804\" y=\"425\" width=\"36\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"55\" x=\"866\" y=\"170\" width=\"41\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"56\" x=\"36\" y=\"595\" width=\"35\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"57\" x=\"117\" y=\"425\" width=\"38\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"58\" x=\"691\" y=\"935\" width=\"20\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"15\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"59\" x=\"541\" y=\"935\" width=\"21\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"15\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"60\" x=\"224\" y=\"850\" width=\"31\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"61\" x=\"0\" y=\"680\" width=\"34\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"62\" x=\"0\" y=\"850\" width=\"31\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"63\" x=\"612\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"23\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"64\" x=\"889\" y=\"0\" width=\"48\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"40\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"65\" x=\"488\" y=\"255\" width=\"39\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"66\" x=\"352\" y=\"170\" width=\"43\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"36\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"67\" x=\"967\" y=\"255\" width=\"38\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"68\" x=\"381\" y=\"85\" width=\"45\" height=\"84\" xoffset=\"-7\" yoffset=\"-5\" xadvance=\"35\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"69\" x=\"595\" y=\"680\" width=\"34\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"70\" x=\"629\" y=\"510\" width=\"36\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"71\" x=\"568\" y=\"170\" width=\"42\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"34\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"72\" x=\"0\" y=\"340\" width=\"38\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"73\" x=\"794\" y=\"935\" width=\"19\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"74\" x=\"192\" y=\"85\" width=\"47\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"75\" x=\"282\" y=\"0\" width=\"51\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"36\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"76\" x=\"646\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"77\" x=\"938\" y=\"0\" width=\"47\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"39\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"78\" x=\"234\" y=\"425\" width=\"38\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"79\" x=\"195\" y=\"425\" width=\"38\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"80\" x=\"782\" y=\"170\" width=\"41\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"81\" x=\"178\" y=\"0\" width=\"51\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"38\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"82\" x=\"589\" y=\"0\" width=\"49\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"33\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"83\" x=\"220\" y=\"170\" width=\"43\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"34\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"84\" x=\"744\" y=\"85\" width=\"44\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"85\" x=\"666\" y=\"510\" width=\"36\" height=\"84\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"86\" x=\"176\" y=\"170\" width=\"43\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"87\" x=\"437\" y=\"0\" width=\"50\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"40\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"88\" x=\"819\" y=\"340\" width=\"38\" height=\"84\" xoffset=\"-7\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"89\" x=\"728\" y=\"255\" width=\"39\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"90\" x=\"0\" y=\"255\" width=\"40\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"91\" x=\"680\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"92\" x=\"64\" y=\"850\" width=\"31\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"16\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"93\" x=\"455\" y=\"680\" width=\"34\" height=\"84\" xoffset=\"-9\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"94\" x=\"714\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"26\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"95\" x=\"691\" y=\"425\" width=\"37\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"96\" x=\"352\" y=\"935\" width=\"23\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"15\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"97\" x=\"577\" y=\"425\" width=\"37\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"98\" x=\"748\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"99\" x=\"288\" y=\"850\" width=\"31\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"100\" x=\"884\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"101\" x=\"918\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"102\" x=\"195\" y=\"340\" width=\"38\" height=\"84\" xoffset=\"-7\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"103\" x=\"350\" y=\"680\" width=\"34\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"104\" x=\"108\" y=\"595\" width=\"35\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"26\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"105\" x=\"607\" y=\"935\" width=\"20\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"106\" x=\"225\" y=\"935\" width=\"25\" height=\"84\" xoffset=\"-8\" yoffset=\"-5\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"107\" x=\"370\" y=\"510\" width=\"36\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"26\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"108\" x=\"563\" y=\"935\" width=\"21\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"109\" x=\"654\" y=\"85\" width=\"44\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"36\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"110\" x=\"874\" y=\"680\" width=\"33\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"111\" x=\"144\" y=\"595\" width=\"35\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"112\" x=\"296\" y=\"510\" width=\"36\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"113\" x=\"897\" y=\"340\" width=\"38\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"114\" x=\"415\" y=\"850\" width=\"30\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"115\" x=\"555\" y=\"510\" width=\"36\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"116\" x=\"863\" y=\"595\" width=\"34\" height=\"84\" xoffset=\"-7\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"117\" x=\"180\" y=\"595\" width=\"35\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"118\" x=\"245\" y=\"680\" width=\"34\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"119\" x=\"699\" y=\"85\" width=\"44\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"35\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"120\" x=\"942\" y=\"680\" width=\"33\" height=\"84\" xoffset=\"-7\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"121\" x=\"0\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"122\" x=\"78\" y=\"340\" width=\"38\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"123\" x=\"192\" y=\"850\" width=\"31\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"23\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"124\" x=\"472\" y=\"935\" width=\"22\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"125\" x=\"384\" y=\"850\" width=\"30\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"126\" x=\"210\" y=\"680\" width=\"34\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"160\" x=\"1008\" y=\"850\" width=\"13\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"14\" page=\"0\" chnl=\"15\" />												"
	<< "	<char id=\"161\" x=\"628\" y=\"935\" width=\"20\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"15\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"162\" x=\"933\" y=\"595\" width=\"34\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"26\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"163\" x=\"315\" y=\"680\" width=\"34\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"26\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"164\" x=\"175\" y=\"680\" width=\"34\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"165\" x=\"148\" y=\"510\" width=\"36\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"166\" x=\"649\" y=\"935\" width=\"20\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"167\" x=\"34\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"26\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"168\" x=\"996\" y=\"510\" width=\"27\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"169\" x=\"264\" y=\"170\" width=\"43\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"36\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"170\" x=\"598\" y=\"850\" width=\"29\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"171\" x=\"878\" y=\"425\" width=\"36\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"172\" x=\"828\" y=\"595\" width=\"34\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"173\" x=\"747\" y=\"850\" width=\"28\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"174\" x=\"308\" y=\"170\" width=\"43\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"34\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"175\" x=\"568\" y=\"850\" width=\"29\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"176\" x=\"538\" y=\"850\" width=\"29\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"19\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"177\" x=\"407\" y=\"510\" width=\"36\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"178\" x=\"32\" y=\"850\" width=\"31\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"179\" x=\"991\" y=\"170\" width=\"32\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"180\" x=\"448\" y=\"935\" width=\"23\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"15\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"181\" x=\"185\" y=\"510\" width=\"36\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"182\" x=\"82\" y=\"255\" width=\"40\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"33\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"183\" x=\"774\" y=\"935\" width=\"19\" height=\"84\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"15\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"184\" x=\"495\" y=\"935\" width=\"22\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"17\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"185\" x=\"733\" y=\"935\" width=\"20\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"186\" x=\"628\" y=\"850\" width=\"29\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"187\" x=\"432\" y=\"595\" width=\"35\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"188\" x=\"839\" y=\"0\" width=\"49\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"40\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"189\" x=\"386\" y=\"0\" width=\"50\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"41\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"190\" x=\"0\" y=\"0\" width=\"60\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"50\" page=\"0\" chnl=\"15\" />														"
	<< "	<char id=\"191\" x=\"508\" y=\"850\" width=\"29\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"20\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"192\" x=\"608\" y=\"255\" width=\"39\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"193\" x=\"368\" y=\"255\" width=\"39\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"194\" x=\"408\" y=\"255\" width=\"39\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"195\" x=\"568\" y=\"255\" width=\"39\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"196\" x=\"448\" y=\"255\" width=\"39\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"197\" x=\"888\" y=\"255\" width=\"39\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"198\" x=\"144\" y=\"85\" width=\"47\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"36\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"199\" x=\"928\" y=\"255\" width=\"38\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"200\" x=\"700\" y=\"680\" width=\"34\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"201\" x=\"735\" y=\"680\" width=\"34\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"202\" x=\"770\" y=\"680\" width=\"34\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"203\" x=\"989\" y=\"425\" width=\"34\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"204\" x=\"302\" y=\"935\" width=\"24\" height=\"84\" xoffset=\"-8\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"205\" x=\"400\" y=\"935\" width=\"23\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"206\" x=\"86\" y=\"935\" width=\"27\" height=\"84\" xoffset=\"-7\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"207\" x=\"892\" y=\"850\" width=\"28\" height=\"84\" xoffset=\"-8\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"208\" x=\"689\" y=\"0\" width=\"49\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"39\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"209\" x=\"0\" y=\"425\" width=\"38\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"210\" x=\"156\" y=\"425\" width=\"38\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"211\" x=\"858\" y=\"340\" width=\"38\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"212\" x=\"780\" y=\"340\" width=\"38\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"213\" x=\"624\" y=\"340\" width=\"38\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"214\" x=\"663\" y=\"340\" width=\"38\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"215\" x=\"352\" y=\"850\" width=\"31\" height=\"84\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"216\" x=\"654\" y=\"170\" width=\"42\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"34\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"217\" x=\"259\" y=\"510\" width=\"36\" height=\"84\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"218\" x=\"952\" y=\"425\" width=\"36\" height=\"84\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"219\" x=\"777\" y=\"510\" width=\"36\" height=\"84\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"220\" x=\"444\" y=\"510\" width=\"36\" height=\"84\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"221\" x=\"688\" y=\"255\" width=\"39\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"222\" x=\"653\" y=\"425\" width=\"37\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"29\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"223\" x=\"507\" y=\"340\" width=\"38\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"224\" x=\"501\" y=\"425\" width=\"37\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"225\" x=\"539\" y=\"425\" width=\"37\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"226\" x=\"311\" y=\"425\" width=\"37\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"227\" x=\"986\" y=\"0\" width=\"37\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"228\" x=\"273\" y=\"425\" width=\"37\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"229\" x=\"463\" y=\"425\" width=\"37\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"230\" x=\"334\" y=\"85\" width=\"46\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"36\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"231\" x=\"128\" y=\"850\" width=\"31\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"232\" x=\"102\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"233\" x=\"136\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"234\" x=\"170\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"235\" x=\"204\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"236\" x=\"198\" y=\"935\" width=\"26\" height=\"84\" xoffset=\"-10\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />												"
	<< "	<char id=\"237\" x=\"424\" y=\"935\" width=\"23\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"238\" x=\"170\" y=\"935\" width=\"27\" height=\"84\" xoffset=\"-8\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"239\" x=\"950\" y=\"850\" width=\"28\" height=\"84\" xoffset=\"-9\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"240\" x=\"702\" y=\"340\" width=\"38\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"241\" x=\"238\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"242\" x=\"612\" y=\"595\" width=\"35\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"243\" x=\"576\" y=\"595\" width=\"35\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"244\" x=\"540\" y=\"595\" width=\"35\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"245\" x=\"504\" y=\"595\" width=\"35\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"246\" x=\"468\" y=\"595\" width=\"35\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"247\" x=\"968\" y=\"595\" width=\"34\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"248\" x=\"222\" y=\"510\" width=\"36\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"249\" x=\"396\" y=\"595\" width=\"35\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"250\" x=\"360\" y=\"595\" width=\"35\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"251\" x=\"288\" y=\"595\" width=\"35\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"252\" x=\"252\" y=\"595\" width=\"35\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"253\" x=\"272\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"254\" x=\"898\" y=\"595\" width=\"34\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"26\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"255\" x=\"340\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"256\" x=\"648\" y=\"255\" width=\"39\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"257\" x=\"729\" y=\"425\" width=\"37\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"258\" x=\"528\" y=\"255\" width=\"39\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"259\" x=\"387\" y=\"425\" width=\"37\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"260\" x=\"41\" y=\"255\" width=\"40\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"261\" x=\"349\" y=\"425\" width=\"37\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"262\" x=\"273\" y=\"340\" width=\"38\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"263\" x=\"320\" y=\"850\" width=\"31\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"264\" x=\"234\" y=\"340\" width=\"38\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"265\" x=\"985\" y=\"765\" width=\"31\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"266\" x=\"156\" y=\"340\" width=\"38\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"267\" x=\"96\" y=\"850\" width=\"31\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"268\" x=\"117\" y=\"340\" width=\"38\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"269\" x=\"160\" y=\"850\" width=\"31\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"270\" x=\"427\" y=\"85\" width=\"45\" height=\"84\" xoffset=\"-7\" yoffset=\"-5\" xadvance=\"35\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"271\" x=\"950\" y=\"170\" width=\"40\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"272\" x=\"789\" y=\"0\" width=\"49\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"39\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"273\" x=\"333\" y=\"510\" width=\"36\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"274\" x=\"280\" y=\"680\" width=\"34\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"275\" x=\"374\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"276\" x=\"385\" y=\"680\" width=\"34\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"277\" x=\"408\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"278\" x=\"420\" y=\"680\" width=\"34\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"279\" x=\"442\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"280\" x=\"490\" y=\"680\" width=\"34\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"281\" x=\"476\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"282\" x=\"630\" y=\"680\" width=\"34\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"283\" x=\"510\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"284\" x=\"525\" y=\"170\" width=\"42\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"34\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"285\" x=\"560\" y=\"680\" width=\"34\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"286\" x=\"697\" y=\"170\" width=\"42\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"34\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"287\" x=\"35\" y=\"680\" width=\"34\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"288\" x=\"482\" y=\"170\" width=\"42\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"34\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"289\" x=\"70\" y=\"680\" width=\"34\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"290\" x=\"439\" y=\"170\" width=\"42\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"34\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"291\" x=\"105\" y=\"680\" width=\"34\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"292\" x=\"468\" y=\"340\" width=\"38\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"293\" x=\"720\" y=\"595\" width=\"35\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"26\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"294\" x=\"473\" y=\"85\" width=\"45\" height=\"84\" xoffset=\"-8\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"295\" x=\"328\" y=\"255\" width=\"39\" height=\"84\" xoffset=\"-8\" yoffset=\"-5\" xadvance=\"26\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"296\" x=\"863\" y=\"850\" width=\"28\" height=\"84\" xoffset=\"-8\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"297\" x=\"658\" y=\"850\" width=\"29\" height=\"84\" xoffset=\"-9\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"298\" x=\"921\" y=\"850\" width=\"28\" height=\"84\" xoffset=\"-8\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"299\" x=\"834\" y=\"850\" width=\"28\" height=\"84\" xoffset=\"-9\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"300\" x=\"805\" y=\"850\" width=\"28\" height=\"84\" xoffset=\"-8\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"301\" x=\"776\" y=\"850\" width=\"28\" height=\"84\" xoffset=\"-9\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"302\" x=\"376\" y=\"935\" width=\"23\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"303\" x=\"327\" y=\"935\" width=\"24\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"304\" x=\"754\" y=\"935\" width=\"19\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"305\" x=\"670\" y=\"935\" width=\"20\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"306\" x=\"120\" y=\"0\" width=\"57\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"42\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"307\" x=\"578\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"308\" x=\"48\" y=\"85\" width=\"47\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"309\" x=\"114\" y=\"935\" width=\"27\" height=\"84\" xoffset=\"-10\" yoffset=\"-5\" xadvance=\"13\" page=\"0\" chnl=\"15\" />												"
	<< "	<char id=\"310\" x=\"334\" y=\"0\" width=\"51\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"36\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"311\" x=\"111\" y=\"510\" width=\"36\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"26\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"312\" x=\"324\" y=\"595\" width=\"35\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"26\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"313\" x=\"68\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"314\" x=\"585\" y=\"935\" width=\"21\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"315\" x=\"782\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"316\" x=\"518\" y=\"935\" width=\"22\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"12\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"317\" x=\"816\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"318\" x=\"0\" y=\"935\" width=\"28\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"15\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"319\" x=\"850\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"320\" x=\"29\" y=\"935\" width=\"28\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"18\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"321\" x=\"123\" y=\"255\" width=\"40\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"322\" x=\"142\" y=\"935\" width=\"27\" height=\"84\" xoffset=\"-8\" yoffset=\"-5\" xadvance=\"14\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"323\" x=\"39\" y=\"340\" width=\"38\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"324\" x=\"840\" y=\"680\" width=\"33\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"325\" x=\"741\" y=\"340\" width=\"38\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"326\" x=\"908\" y=\"680\" width=\"33\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"327\" x=\"585\" y=\"340\" width=\"38\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"328\" x=\"976\" y=\"680\" width=\"33\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"329\" x=\"164\" y=\"255\" width=\"40\" height=\"84\" xoffset=\"-10\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />												"
	<< "	<char id=\"330\" x=\"425\" y=\"425\" width=\"37\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"331\" x=\"952\" y=\"765\" width=\"32\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"332\" x=\"429\" y=\"340\" width=\"38\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"333\" x=\"648\" y=\"595\" width=\"35\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"334\" x=\"390\" y=\"340\" width=\"38\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"335\" x=\"792\" y=\"595\" width=\"35\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"336\" x=\"312\" y=\"340\" width=\"38\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"337\" x=\"960\" y=\"510\" width=\"35\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"338\" x=\"61\" y=\"0\" width=\"58\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"49\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"339\" x=\"287\" y=\"85\" width=\"46\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"37\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"340\" x=\"639\" y=\"0\" width=\"49\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"33\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"341\" x=\"446\" y=\"850\" width=\"30\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"342\" x=\"539\" y=\"0\" width=\"49\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"33\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"343\" x=\"256\" y=\"850\" width=\"31\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"344\" x=\"739\" y=\"0\" width=\"49\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"33\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"345\" x=\"477\" y=\"850\" width=\"30\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"346\" x=\"0\" y=\"170\" width=\"43\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"34\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"347\" x=\"481\" y=\"510\" width=\"36\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"348\" x=\"923\" y=\"85\" width=\"43\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"34\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"349\" x=\"592\" y=\"510\" width=\"36\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"350\" x=\"879\" y=\"85\" width=\"43\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"34\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"351\" x=\"703\" y=\"510\" width=\"36\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"352\" x=\"44\" y=\"170\" width=\"43\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"34\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"353\" x=\"767\" y=\"425\" width=\"36\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"354\" x=\"609\" y=\"85\" width=\"44\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"355\" x=\"525\" y=\"680\" width=\"34\" height=\"84\" xoffset=\"-7\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"356\" x=\"519\" y=\"85\" width=\"44\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"357\" x=\"908\" y=\"170\" width=\"41\" height=\"84\" xoffset=\"-7\" yoffset=\"-5\" xadvance=\"26\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"358\" x=\"834\" y=\"85\" width=\"44\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"359\" x=\"665\" y=\"680\" width=\"34\" height=\"84\" xoffset=\"-7\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"360\" x=\"915\" y=\"425\" width=\"36\" height=\"84\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"361\" x=\"684\" y=\"595\" width=\"35\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"362\" x=\"851\" y=\"510\" width=\"36\" height=\"84\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"363\" x=\"756\" y=\"595\" width=\"35\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"364\" x=\"0\" y=\"510\" width=\"36\" height=\"84\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"365\" x=\"216\" y=\"595\" width=\"35\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"366\" x=\"37\" y=\"510\" width=\"36\" height=\"84\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"367\" x=\"924\" y=\"510\" width=\"35\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"368\" x=\"74\" y=\"510\" width=\"36\" height=\"84\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"369\" x=\"0\" y=\"595\" width=\"35\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"370\" x=\"39\" y=\"425\" width=\"38\" height=\"84\" xoffset=\"-2\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"371\" x=\"72\" y=\"595\" width=\"35\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"372\" x=\"488\" y=\"0\" width=\"50\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"40\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"373\" x=\"789\" y=\"85\" width=\"44\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"35\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"374\" x=\"768\" y=\"255\" width=\"39\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"375\" x=\"306\" y=\"765\" width=\"33\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"376\" x=\"808\" y=\"255\" width=\"39\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"31\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"377\" x=\"205\" y=\"255\" width=\"40\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"378\" x=\"975\" y=\"340\" width=\"38\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"379\" x=\"246\" y=\"255\" width=\"40\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"380\" x=\"546\" y=\"340\" width=\"38\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"381\" x=\"287\" y=\"255\" width=\"40\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"382\" x=\"78\" y=\"425\" width=\"38\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"383\" x=\"718\" y=\"850\" width=\"28\" height=\"84\" xoffset=\"-3\" yoffset=\"-5\" xadvance=\"14\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"402\" x=\"518\" y=\"510\" width=\"36\" height=\"84\" xoffset=\"-9\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"506\" x=\"848\" y=\"255\" width=\"39\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"507\" x=\"615\" y=\"425\" width=\"37\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"508\" x=\"0\" y=\"85\" width=\"47\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"36\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"509\" x=\"240\" y=\"85\" width=\"46\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"36\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"510\" x=\"611\" y=\"170\" width=\"42\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"34\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"511\" x=\"740\" y=\"510\" width=\"36\" height=\"84\" xoffset=\"-4\" yoffset=\"-5\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"536\" x=\"132\" y=\"170\" width=\"43\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"34\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"537\" x=\"814\" y=\"510\" width=\"36\" height=\"84\" xoffset=\"-5\" yoffset=\"-5\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"538\" x=\"564\" y=\"85\" width=\"44\" height=\"84\" xoffset=\"-6\" yoffset=\"-5\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"539\" x=\"805\" y=\"680\" width=\"34\" height=\"84\" xoffset=\"-7\" yoffset=\"-5\" xadvance=\"21\" page=\"0\" chnl=\"15\" />													"
	<< "	<char id=\"567\" x=\"251\" y=\"935\" width=\"25\" height=\"84\" xoffset=\"-8\" yoffset=\"-5\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"
	<< "	</chars>																																												"
	<< "	<kernings count=\"2109\">																																								"
	<< "	<kerning first=\"65\" second=\"84\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"65\" second=\"86\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"65\" second=\"47\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"65\" second=\"92\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"65\" second=\"39\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"65\" second=\"34\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"373\" second=\"44\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"373\" second=\"46\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"65\" second=\"35\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"65\" second=\"42\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"65\" second=\"174\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"372\" second=\"358\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"65\" second=\"354\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"65\" second=\"538\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"65\" second=\"356\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"65\" second=\"358\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"372\" second=\"356\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"372\" second=\"538\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"372\" second=\"354\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"372\" second=\"44\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"66\" second=\"84\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"66\" second=\"86\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"66\" second=\"88\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"66\" second=\"93\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"66\" second=\"92\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"66\" second=\"95\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"372\" second=\"46\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"66\" second=\"46\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"66\" second=\"44\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"372\" second=\"95\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"66\" second=\"39\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"66\" second=\"34\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"372\" second=\"84\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"358\" second=\"380\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"378\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"373\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"66\" second=\"35\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"66\" second=\"42\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"66\" second=\"174\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"372\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"66\" second=\"354\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"66\" second=\"538\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"66\" second=\"356\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"66\" second=\"358\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"358\" second=\"371\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"369\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"367\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"365\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"363\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"361\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"359\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"67\" second=\"80\" amount=\"2\" />																																		"
	<< "	<kerning first=\"67\" second=\"125\" amount=\"2\" />																																	"
	<< "	<kerning first=\"67\" second=\"124\" amount=\"2\" />																																	"
	<< "	<kerning first=\"67\" second=\"166\" amount=\"2\" />																																	"
	<< "	<kerning first=\"67\" second=\"92\" amount=\"2\" />																																		"
	<< "	<kerning first=\"67\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"357\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"67\" second=\"63\" amount=\"2\" />																																		"
	<< "	<kerning first=\"67\" second=\"39\" amount=\"3\" />																																		"
	<< "	<kerning first=\"67\" second=\"34\" amount=\"3\" />																																		"
	<< "	<kerning first=\"67\" second=\"42\" amount=\"4\" />																																		"
	<< "	<kerning first=\"67\" second=\"64\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"67\" second=\"174\" amount=\"4\" />																																	"
	<< "	<kerning first=\"358\" second=\"539\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"67\" second=\"238\" amount=\"4\" />																																	"
	<< "	<kerning first=\"358\" second=\"355\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"358\" second=\"537\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"68\" second=\"67\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"68\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"536\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"68\" second=\"46\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"68\" second=\"44\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"351\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"358\" second=\"350\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"358\" second=\"349\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"68\" second=\"42\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"70\" second=\"124\" amount=\"2\" />																																	"
	<< "	<kerning first=\"70\" second=\"166\" amount=\"2\" />																																	"
	<< "	<kerning first=\"70\" second=\"92\" amount=\"2\" />																																		"
	<< "	<kerning first=\"70\" second=\"45\" amount=\"2\" />																																		"
	<< "	<kerning first=\"358\" second=\"348\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"358\" second=\"347\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"358\" second=\"346\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"70\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"345\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"70\" second=\"46\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"70\" second=\"44\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"358\" second=\"343\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"70\" second=\"63\" amount=\"2\" />																																		"
	<< "	<kerning first=\"70\" second=\"39\" amount=\"2\" />																																		"
	<< "	<kerning first=\"70\" second=\"34\" amount=\"3\" />																																		"
	<< "	<kerning first=\"358\" second=\"341\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"511\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"337\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"335\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"70\" second=\"42\" amount=\"3\" />																																		"
	<< "	<kerning first=\"70\" second=\"174\" amount=\"2\" />																																	"
	<< "	<kerning first=\"358\" second=\"333\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"74\" second=\"84\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"74\" second=\"99\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"74\" second=\"100\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"101\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"103\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"111\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"113\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"115\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"117\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"74\" second=\"119\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"74\" second=\"41\" amount=\"3\" />																																		"
	<< "	<kerning first=\"74\" second=\"93\" amount=\"3\" />																																		"
	<< "	<kerning first=\"74\" second=\"125\" amount=\"3\" />																																	"
	<< "	<kerning first=\"74\" second=\"46\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"44\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"331\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"74\" second=\"33\" amount=\"2\" />																																		"
	<< "	<kerning first=\"74\" second=\"63\" amount=\"8\" />																																		"
	<< "	<kerning first=\"74\" second=\"39\" amount=\"9\" />																																		"
	<< "	<kerning first=\"74\" second=\"34\" amount=\"9\" />																																		"
	<< "	<kerning first=\"358\" second=\"328\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"326\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"324\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"312\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"74\" second=\"231\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"232\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"233\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"234\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"235\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"240\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"242\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"243\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"244\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"245\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"246\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"248\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"339\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"353\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"249\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"74\" second=\"250\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"74\" second=\"251\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"74\" second=\"252\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"74\" second=\"263\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"265\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"269\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"271\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"273\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"275\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"277\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"279\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"281\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"283\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"285\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"287\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"291\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"333\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"335\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"337\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"511\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"347\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"349\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"351\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"537\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"74\" second=\"354\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"74\" second=\"538\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"74\" second=\"356\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"74\" second=\"358\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"74\" second=\"361\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"74\" second=\"363\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"74\" second=\"365\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"74\" second=\"367\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"74\" second=\"369\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"74\" second=\"371\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"74\" second=\"373\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"358\" second=\"291\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"290\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"289\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"288\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"287\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"286\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"285\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"284\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"283\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"281\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"279\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"277\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"275\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"273\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"271\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"269\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"268\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"266\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"265\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"264\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"263\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"262\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"509\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"508\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"507\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"506\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"261\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"260\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"75\" second=\"67\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"75\" second=\"71\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"75\" second=\"79\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"75\" second=\"81\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"75\" second=\"84\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"75\" second=\"99\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"100\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"101\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"103\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"106\" amount=\"2\" />																																	"
	<< "	<kerning first=\"75\" second=\"111\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"113\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"95\" amount=\"4\" />																																		"
	<< "	<kerning first=\"358\" second=\"259\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"75\" second=\"46\" amount=\"6\" />																																		"
	<< "	<kerning first=\"75\" second=\"44\" amount=\"6\" />																																		"
	<< "	<kerning first=\"75\" second=\"58\" amount=\"6\" />																																		"
	<< "	<kerning first=\"75\" second=\"59\" amount=\"6\" />																																		"
	<< "	<kerning first=\"358\" second=\"258\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"257\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"75\" second=\"33\" amount=\"6\" />																																		"
	<< "	<kerning first=\"75\" second=\"63\" amount=\"5\" />																																		"
	<< "	<kerning first=\"358\" second=\"256\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"382\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"358\" second=\"253\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"358\" second=\"252\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"358\" second=\"251\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"358\" second=\"250\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"75\" second=\"171\" amount=\"5\" />																																	"
	<< "	<kerning first=\"75\" second=\"187\" amount=\"5\" />																																	"
	<< "	<kerning first=\"75\" second=\"208\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"75\" second=\"210\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"75\" second=\"211\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"75\" second=\"212\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"75\" second=\"213\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"75\" second=\"214\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"75\" second=\"216\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"75\" second=\"338\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"75\" second=\"222\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"75\" second=\"231\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"232\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"233\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"234\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"235\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"240\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"242\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"243\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"244\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"245\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"246\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"248\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"339\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"262\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"75\" second=\"263\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"264\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"75\" second=\"265\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"268\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"75\" second=\"269\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"270\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"75\" second=\"271\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"272\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"75\" second=\"273\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"275\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"277\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"279\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"281\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"283\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"284\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"75\" second=\"285\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"286\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"75\" second=\"287\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"290\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"75\" second=\"291\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"332\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"75\" second=\"333\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"334\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"75\" second=\"335\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"336\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"75\" second=\"337\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"510\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"75\" second=\"511\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"75\" second=\"354\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"75\" second=\"356\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"75\" second=\"358\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"358\" second=\"249\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"358\" second=\"353\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"358\" second=\"339\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"248\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"246\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"245\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"244\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"243\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"242\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"241\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"305\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"240\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"235\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"234\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"233\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"232\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"358\" second=\"231\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"230\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"229\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"228\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"227\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"226\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"225\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"224\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"358\" second=\"352\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"358\" second=\"321\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"358\" second=\"199\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"197\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"196\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"195\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"76\" second=\"84\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"76\" second=\"86\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"76\" second=\"87\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"76\" second=\"92\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"76\" second=\"39\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"76\" second=\"34\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"358\" second=\"194\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"193\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"76\" second=\"42\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"76\" second=\"174\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"358\" second=\"192\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"76\" second=\"354\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"76\" second=\"538\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"76\" second=\"356\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"76\" second=\"358\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"76\" second=\"372\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"174\" amount=\"4\" />																																	"
	<< "	<kerning first=\"358\" second=\"64\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"42\" amount=\"4\" />																																	"
	<< "	<kerning first=\"358\" second=\"187\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"171\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"358\" second=\"34\" amount=\"4\" />																																	"
	<< "	<kerning first=\"358\" second=\"39\" amount=\"4\" />																																	"
	<< "	<kerning first=\"79\" second=\"67\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"79\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"59\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"79\" second=\"46\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"79\" second=\"44\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"58\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"358\" second=\"44\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"358\" second=\"46\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"80\" second=\"65\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"80\" second=\"67\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"80\" second=\"38\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"97\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"99\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"80\" second=\"100\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"101\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"103\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"80\" second=\"111\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"113\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"80\" second=\"115\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"80\" second=\"47\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"80\" second=\"45\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"358\" second=\"95\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"358\" second=\"45\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"358\" second=\"47\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"95\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"358\" second=\"122\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"80\" second=\"46\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"80\" second=\"44\" amount=\"-13\" />																																	"
	<< "	<kerning first=\"358\" second=\"121\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"80\" second=\"63\" amount=\"2\" />																																		"
	<< "	<kerning first=\"358\" second=\"120\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"358\" second=\"119\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"118\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"80\" second=\"171\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"80\" second=\"42\" amount=\"2\" />																																		"
	<< "	<kerning first=\"80\" second=\"64\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"80\" second=\"192\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"80\" second=\"193\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"80\" second=\"194\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"80\" second=\"195\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"80\" second=\"196\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"80\" second=\"197\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"80\" second=\"199\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"80\" second=\"321\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"80\" second=\"224\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"225\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"80\" second=\"226\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"227\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"228\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"229\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"80\" second=\"230\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"231\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"232\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"233\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"234\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"235\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"240\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"242\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"243\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"244\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"245\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"246\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"248\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"339\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"353\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"80\" second=\"256\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"80\" second=\"257\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"258\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"80\" second=\"259\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"260\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"80\" second=\"261\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"506\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"80\" second=\"507\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"508\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"80\" second=\"509\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"262\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"80\" second=\"263\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"264\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"80\" second=\"265\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"266\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"80\" second=\"268\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"80\" second=\"269\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"271\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"273\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"275\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"277\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"279\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"281\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"283\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"285\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"80\" second=\"287\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"80\" second=\"289\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"80\" second=\"291\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"80\" second=\"333\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"335\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"337\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"511\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"80\" second=\"347\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"80\" second=\"349\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"80\" second=\"351\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"80\" second=\"537\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"358\" second=\"117\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"116\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"358\" second=\"115\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"358\" second=\"114\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"113\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"112\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"111\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"110\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"109\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"106\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"105\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"104\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"103\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"102\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"358\" second=\"101\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"100\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"99\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"97\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"81\" second=\"67\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"81\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"88\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"81\" second=\"46\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"81\" second=\"44\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"87\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"86\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"358\" second=\"83\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"82\" second=\"67\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"82\" second=\"95\" amount=\"4\" />																																		"
	<< "	<kerning first=\"358\" second=\"71\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"82\" second=\"46\" amount=\"6\" />																																		"
	<< "	<kerning first=\"82\" second=\"44\" amount=\"5\" />																																		"
	<< "	<kerning first=\"82\" second=\"58\" amount=\"4\" />																																		"
	<< "	<kerning first=\"82\" second=\"59\" amount=\"4\" />																																		"
	<< "	<kerning first=\"358\" second=\"67\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"82\" second=\"33\" amount=\"5\" />																																		"
	<< "	<kerning first=\"82\" second=\"63\" amount=\"5\" />																																		"
	<< "	<kerning first=\"358\" second=\"65\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"357\" second=\"318\" amount=\"3\" />																																	"
	<< "	<kerning first=\"357\" second=\"108\" amount=\"4\" />																																	"
	<< "	<kerning first=\"82\" second=\"171\" amount=\"2\" />																																	"
	<< "	<kerning first=\"82\" second=\"199\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"82\" second=\"262\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"82\" second=\"264\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"82\" second=\"266\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"82\" second=\"268\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"83\" second=\"84\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"83\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"357\" second=\"107\" amount=\"2\" />																																	"
	<< "	<kerning first=\"83\" second=\"46\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"83\" second=\"44\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"357\" second=\"106\" amount=\"4\" />																																	"
	<< "	<kerning first=\"357\" second=\"105\" amount=\"2\" />																																	"
	<< "	<kerning first=\"357\" second=\"104\" amount=\"2\" />																																	"
	<< "	<kerning first=\"357\" second=\"98\" amount=\"2\" />																																	"
	<< "	<kerning first=\"356\" second=\"380\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"83\" second=\"42\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"83\" second=\"354\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"83\" second=\"356\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"83\" second=\"358\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"356\" second=\"378\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"373\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"372\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"371\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"65\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"67\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"71\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"83\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"84\" second=\"86\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"84\" second=\"87\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"88\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"84\" second=\"97\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"99\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"100\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"101\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"102\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"84\" second=\"103\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"104\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"105\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"106\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"109\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"110\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"111\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"112\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"113\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"114\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"115\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"84\" second=\"116\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"84\" second=\"117\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"118\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"119\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"120\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"84\" second=\"121\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"122\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"47\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"84\" second=\"45\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"356\" second=\"369\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"367\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"365\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"95\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"356\" second=\"363\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"46\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"84\" second=\"44\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"84\" second=\"58\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"84\" second=\"59\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"356\" second=\"361\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"39\" amount=\"4\" />																																		"
	<< "	<kerning first=\"84\" second=\"34\" amount=\"4\" />																																		"
	<< "	<kerning first=\"356\" second=\"359\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"356\" second=\"357\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"356\" second=\"539\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"356\" second=\"355\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"84\" second=\"171\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"84\" second=\"187\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"42\" amount=\"4\" />																																		"
	<< "	<kerning first=\"84\" second=\"64\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"174\" amount=\"4\" />																																	"
	<< "	<kerning first=\"356\" second=\"537\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"84\" second=\"192\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"193\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"194\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"195\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"196\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"197\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"199\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"321\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"84\" second=\"352\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"84\" second=\"224\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"84\" second=\"225\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"226\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"227\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"228\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"229\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"230\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"231\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"232\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"84\" second=\"233\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"234\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"235\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"240\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"305\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"241\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"242\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"243\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"244\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"245\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"246\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"248\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"339\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"353\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"84\" second=\"249\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"84\" second=\"250\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"251\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"84\" second=\"252\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"84\" second=\"253\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"84\" second=\"382\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"84\" second=\"256\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"257\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"258\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"259\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"260\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"261\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"506\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"507\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"508\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"509\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"262\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"263\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"264\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"265\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"266\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"268\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"269\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"271\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"273\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"275\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"277\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"279\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"281\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"283\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"284\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"285\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"286\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"287\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"288\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"289\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"290\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"291\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"312\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"324\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"326\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"328\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"331\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"333\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"335\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"337\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"511\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"341\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"343\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"345\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"346\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"84\" second=\"347\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"84\" second=\"348\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"84\" second=\"349\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"84\" second=\"350\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"84\" second=\"351\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"84\" second=\"536\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"84\" second=\"537\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"84\" second=\"355\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"84\" second=\"539\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"84\" second=\"357\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"84\" second=\"359\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"84\" second=\"361\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"363\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"365\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"367\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"369\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"371\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"372\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"84\" second=\"373\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"536\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"356\" second=\"351\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"356\" second=\"350\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"356\" second=\"349\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"356\" second=\"348\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"356\" second=\"347\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"84\" second=\"378\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"380\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"346\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"356\" second=\"345\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"343\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"341\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"511\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"337\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"335\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"333\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"331\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"328\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"326\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"324\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"312\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"291\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"290\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"289\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"288\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"287\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"286\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"285\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"284\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"283\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"281\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"279\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"277\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"275\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"273\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"271\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"269\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"268\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"266\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"265\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"264\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"263\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"262\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"509\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"508\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"507\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"506\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"261\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"260\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"259\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"258\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"257\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"256\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"382\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"253\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"356\" second=\"252\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"251\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"250\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"249\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"353\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"356\" second=\"339\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"248\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"246\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"245\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"244\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"86\" second=\"67\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"80\" amount=\"2\" />																																		"
	<< "	<kerning first=\"86\" second=\"84\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"86\" second=\"97\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"99\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"100\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"101\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"103\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"111\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"113\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"115\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"47\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"45\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"243\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"242\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"241\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"86\" second=\"95\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"305\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"86\" second=\"46\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"86\" second=\"44\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"356\" second=\"240\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"86\" second=\"63\" amount=\"2\" />																																		"
	<< "	<kerning first=\"86\" second=\"39\" amount=\"5\" />																																		"
	<< "	<kerning first=\"86\" second=\"34\" amount=\"5\" />																																		"
	<< "	<kerning first=\"356\" second=\"235\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"234\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"233\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"232\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"356\" second=\"231\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"86\" second=\"171\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"86\" second=\"42\" amount=\"6\" />																																		"
	<< "	<kerning first=\"86\" second=\"64\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"174\" amount=\"5\" />																																	"
	<< "	<kerning first=\"356\" second=\"230\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"86\" second=\"224\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"225\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"226\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"227\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"228\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"229\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"230\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"231\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"232\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"233\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"234\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"235\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"240\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"242\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"243\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"244\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"245\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"246\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"248\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"339\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"353\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"257\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"259\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"261\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"507\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"509\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"263\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"265\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"269\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"271\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"273\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"275\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"277\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"279\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"281\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"283\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"285\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"287\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"289\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"291\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"333\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"335\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"337\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"511\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"347\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"349\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"351\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"537\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"86\" second=\"354\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"86\" second=\"538\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"86\" second=\"356\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"86\" second=\"358\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"229\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"228\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"227\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"226\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"225\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"224\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"356\" second=\"352\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"356\" second=\"321\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"356\" second=\"199\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"197\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"196\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"195\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"194\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"193\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"192\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"174\" amount=\"4\" />																																	"
	<< "	<kerning first=\"356\" second=\"64\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"42\" amount=\"4\" />																																	"
	<< "	<kerning first=\"356\" second=\"187\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"171\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"356\" second=\"34\" amount=\"4\" />																																	"
	<< "	<kerning first=\"356\" second=\"39\" amount=\"4\" />																																	"
	<< "	<kerning first=\"356\" second=\"59\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"87\" second=\"84\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"87\" second=\"95\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"356\" second=\"58\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"87\" second=\"46\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"87\" second=\"44\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"44\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"356\" second=\"46\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"356\" second=\"95\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"356\" second=\"45\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"87\" second=\"354\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"87\" second=\"538\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"87\" second=\"356\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"87\" second=\"358\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"47\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"356\" second=\"122\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"121\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"120\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"88\" second=\"84\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"119\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"118\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"88\" second=\"354\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"88\" second=\"538\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"88\" second=\"356\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"88\" second=\"358\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"117\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"116\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"356\" second=\"115\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"356\" second=\"114\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"98\" second=\"92\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"102\" second=\"102\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"102\" second=\"41\" amount=\"4\" />																																	"
	<< "	<kerning first=\"102\" second=\"93\" amount=\"4\" />																																	"
	<< "	<kerning first=\"102\" second=\"125\" amount=\"4\" />																																	"
	<< "	<kerning first=\"102\" second=\"124\" amount=\"5\" />																																	"
	<< "	<kerning first=\"102\" second=\"166\" amount=\"5\" />																																	"
	<< "	<kerning first=\"102\" second=\"39\" amount=\"6\" />																																	"
	<< "	<kerning first=\"102\" second=\"34\" amount=\"6\" />																																	"
	<< "	<kerning first=\"356\" second=\"113\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"112\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"102\" second=\"42\" amount=\"2\" />																																	"
	<< "	<kerning first=\"102\" second=\"174\" amount=\"4\" />																																	"
	<< "	<kerning first=\"356\" second=\"111\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"107\" second=\"97\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"99\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"100\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"101\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"103\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"111\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"113\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"120\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"92\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"107\" second=\"45\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"110\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"109\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"106\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"105\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"104\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"107\" second=\"224\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"225\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"226\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"227\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"228\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"229\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"230\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"231\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"232\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"233\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"234\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"235\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"240\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"242\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"243\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"244\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"245\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"246\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"248\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"339\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"257\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"259\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"261\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"507\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"509\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"263\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"265\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"269\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"271\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"273\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"275\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"277\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"279\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"281\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"283\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"285\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"287\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"289\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"291\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"333\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"335\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"337\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"511\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"103\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"102\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"356\" second=\"101\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"100\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"99\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"97\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"88\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"87\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"86\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"83\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"356\" second=\"71\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"67\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"65\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"538\" second=\"380\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"378\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"373\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"372\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"538\" second=\"371\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"111\" second=\"92\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"112\" second=\"92\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"112\" second=\"95\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"369\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"112\" second=\"46\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"112\" second=\"44\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"538\" second=\"367\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"365\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"363\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"114\" second=\"113\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"114\" second=\"47\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"114\" second=\"45\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"538\" second=\"361\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"359\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"538\" second=\"357\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"114\" second=\"95\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"538\" second=\"539\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"114\" second=\"46\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"114\" second=\"44\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"538\" second=\"355\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"114\" second=\"39\" amount=\"2\" />																																	"
	<< "	<kerning first=\"114\" second=\"34\" amount=\"2\" />																																	"
	<< "	<kerning first=\"538\" second=\"537\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"538\" second=\"351\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"538\" second=\"349\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"114\" second=\"171\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"114\" second=\"42\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"114\" second=\"231\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"118\" second=\"95\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"538\" second=\"347\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"118\" second=\"46\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"118\" second=\"44\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"538\" second=\"345\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"118\" second=\"63\" amount=\"3\" />																																	"
	<< "	<kerning first=\"118\" second=\"39\" amount=\"3\" />																																	"
	<< "	<kerning first=\"118\" second=\"34\" amount=\"3\" />																																	"
	<< "	<kerning first=\"538\" second=\"343\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"341\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"119\" second=\"46\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"119\" second=\"44\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"538\" second=\"511\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"337\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"335\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"120\" second=\"39\" amount=\"3\" />																																	"
	<< "	<kerning first=\"120\" second=\"34\" amount=\"3\" />																																	"
	<< "	<kerning first=\"47\" second=\"67\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"47\" second=\"99\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"100\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"101\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"103\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"111\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"113\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"47\" second=\"199\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"47\" second=\"231\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"232\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"233\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"234\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"235\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"240\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"242\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"243\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"244\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"245\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"246\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"248\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"339\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"262\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"47\" second=\"263\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"264\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"47\" second=\"265\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"266\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"47\" second=\"268\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"47\" second=\"269\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"271\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"273\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"275\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"277\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"279\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"281\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"283\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"285\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"287\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"291\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"333\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"335\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"337\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"47\" second=\"511\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"538\" second=\"333\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"331\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"328\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"326\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"324\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"312\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"291\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"290\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"538\" second=\"289\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"288\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"538\" second=\"287\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"286\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"538\" second=\"285\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"284\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"538\" second=\"283\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"92\" second=\"84\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"92\" second=\"86\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"92\" second=\"87\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"92\" second=\"113\" amount=\"3\" />																																	"
	<< "	<kerning first=\"92\" second=\"354\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"92\" second=\"538\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"92\" second=\"356\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"92\" second=\"358\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"92\" second=\"372\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"538\" second=\"281\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"279\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"277\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"275\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"273\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"271\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"269\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"45\" second=\"84\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"45\" second=\"86\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"45\" second=\"88\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"45\" second=\"354\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"45\" second=\"538\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"45\" second=\"356\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"45\" second=\"358\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"538\" second=\"268\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"538\" second=\"266\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"538\" second=\"265\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"264\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"538\" second=\"263\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"262\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"538\" second=\"509\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"507\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"261\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"259\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"257\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"382\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"538\" second=\"253\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"538\" second=\"252\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"538\" second=\"251\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"538\" second=\"250\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"249\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"538\" second=\"353\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"538\" second=\"339\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"248\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"246\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"245\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"244\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"243\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"242\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"241\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"305\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"240\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"235\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"234\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"233\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"232\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"538\" second=\"231\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"230\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"229\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"228\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"227\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"226\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"538\" second=\"225\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"224\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"538\" second=\"321\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"538\" second=\"199\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"538\" second=\"174\" amount=\"4\" />																																	"
	<< "	<kerning first=\"538\" second=\"64\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"538\" second=\"42\" amount=\"4\" />																																	"
	<< "	<kerning first=\"538\" second=\"187\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"538\" second=\"171\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"538\" second=\"34\" amount=\"4\" />																																	"
	<< "	<kerning first=\"538\" second=\"39\" amount=\"4\" />																																	"
	<< "	<kerning first=\"95\" second=\"84\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"95\" second=\"118\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"95\" second=\"120\" amount=\"2\" />																																	"
	<< "	<kerning first=\"95\" second=\"354\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"95\" second=\"538\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"95\" second=\"356\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"95\" second=\"358\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"538\" second=\"59\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"538\" second=\"58\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"538\" second=\"44\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"538\" second=\"46\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"538\" second=\"95\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"538\" second=\"45\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"538\" second=\"47\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"538\" second=\"122\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"121\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"120\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"538\" second=\"119\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"118\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"117\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"116\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"538\" second=\"115\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"538\" second=\"114\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"113\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"112\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"111\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"110\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"109\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"106\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"538\" second=\"105\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"538\" second=\"104\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"538\" second=\"103\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"46\" second=\"84\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"46\" second=\"86\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"46\" second=\"87\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"46\" second=\"354\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"46\" second=\"538\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"46\" second=\"356\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"46\" second=\"358\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"46\" second=\"372\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"538\" second=\"102\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"538\" second=\"101\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"100\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"99\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"97\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"88\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"538\" second=\"87\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"538\" second=\"86\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"538\" second=\"71\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"538\" second=\"67\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"380\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"378\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"373\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"372\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"371\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"369\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"367\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"365\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"363\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"361\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"191\" second=\"88\" amount=\"3\" />																																	"
	<< "	<kerning first=\"354\" second=\"359\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"354\" second=\"357\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"354\" second=\"539\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"39\" second=\"67\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"39\" second=\"102\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"39\" second=\"113\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"39\" second=\"199\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"39\" second=\"262\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"39\" second=\"264\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"39\" second=\"266\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"39\" second=\"268\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"34\" second=\"65\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"34\" second=\"67\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"34\" second=\"102\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"34\" second=\"113\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"34\" second=\"120\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"34\" second=\"192\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"34\" second=\"193\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"34\" second=\"194\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"34\" second=\"195\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"34\" second=\"196\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"34\" second=\"197\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"34\" second=\"199\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"34\" second=\"256\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"34\" second=\"258\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"34\" second=\"260\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"34\" second=\"506\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"34\" second=\"508\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"34\" second=\"262\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"34\" second=\"264\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"34\" second=\"266\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"34\" second=\"268\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"355\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"354\" second=\"537\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"354\" second=\"536\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"354\" second=\"351\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"354\" second=\"350\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"354\" second=\"349\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"354\" second=\"348\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"354\" second=\"347\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"354\" second=\"346\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"354\" second=\"345\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"343\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"341\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"511\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"337\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"335\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"333\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"331\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"328\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"326\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"324\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"312\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"291\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"290\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"289\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"288\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"287\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"286\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"285\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"284\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"283\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"281\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"279\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"277\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"275\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"273\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"271\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"269\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"268\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"266\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"265\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"264\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"263\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"262\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"509\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"508\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"507\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"506\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"261\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"260\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"259\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"258\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"257\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"256\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"382\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"354\" second=\"253\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"354\" second=\"252\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"354\" second=\"251\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"354\" second=\"250\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"249\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"354\" second=\"353\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"354\" second=\"339\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"248\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"246\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"245\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"244\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"243\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"242\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"241\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"305\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"240\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"235\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"234\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"233\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"232\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"354\" second=\"231\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"230\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"229\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"228\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"227\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"226\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"225\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"224\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"354\" second=\"352\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"354\" second=\"321\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"354\" second=\"199\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"197\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"196\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"195\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"194\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"193\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"192\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"174\" amount=\"4\" />																																	"
	<< "	<kerning first=\"354\" second=\"64\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"42\" amount=\"4\" />																																	"
	<< "	<kerning first=\"354\" second=\"187\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"171\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"354\" second=\"34\" amount=\"4\" />																																	"
	<< "	<kerning first=\"354\" second=\"39\" amount=\"4\" />																																	"
	<< "	<kerning first=\"354\" second=\"59\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"354\" second=\"58\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"354\" second=\"44\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"354\" second=\"46\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"354\" second=\"95\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"354\" second=\"45\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"354\" second=\"47\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"354\" second=\"122\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"121\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"120\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"354\" second=\"119\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"118\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"117\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"116\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"354\" second=\"115\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"354\" second=\"114\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"113\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"112\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"111\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"110\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"109\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"106\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"105\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"104\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"103\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"102\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"354\" second=\"101\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"100\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"99\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"97\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"88\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"354\" second=\"87\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"86\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"354\" second=\"83\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"354\" second=\"71\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"67\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"65\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"383\" second=\"44\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"383\" second=\"46\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"536\" second=\"358\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"536\" second=\"356\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"536\" second=\"354\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"536\" second=\"42\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"536\" second=\"44\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"536\" second=\"46\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"536\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"536\" second=\"84\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"350\" second=\"358\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"350\" second=\"356\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"350\" second=\"354\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"350\" second=\"42\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"350\" second=\"44\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"350\" second=\"46\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"350\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"350\" second=\"84\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"348\" second=\"358\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"348\" second=\"356\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"348\" second=\"354\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"348\" second=\"42\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"348\" second=\"44\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"348\" second=\"46\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"348\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"348\" second=\"84\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"346\" second=\"358\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"346\" second=\"356\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"346\" second=\"354\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"346\" second=\"42\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"346\" second=\"44\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"346\" second=\"46\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"346\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"346\" second=\"84\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"345\" second=\"231\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"345\" second=\"42\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"345\" second=\"171\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"345\" second=\"34\" amount=\"2\" />																																	"
	<< "	<kerning first=\"345\" second=\"39\" amount=\"2\" />																																	"
	<< "	<kerning first=\"345\" second=\"44\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"345\" second=\"46\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"345\" second=\"95\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"345\" second=\"45\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"345\" second=\"47\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"345\" second=\"113\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"344\" second=\"268\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"344\" second=\"266\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"344\" second=\"264\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"344\" second=\"262\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"344\" second=\"199\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"344\" second=\"171\" amount=\"2\" />																																	"
	<< "	<kerning first=\"344\" second=\"63\" amount=\"5\" />																																	"
	<< "	<kerning first=\"344\" second=\"33\" amount=\"5\" />																																	"
	<< "	<kerning first=\"344\" second=\"59\" amount=\"4\" />																																	"
	<< "	<kerning first=\"344\" second=\"58\" amount=\"4\" />																																	"
	<< "	<kerning first=\"344\" second=\"44\" amount=\"5\" />																																	"
	<< "	<kerning first=\"344\" second=\"46\" amount=\"6\" />																																	"
	<< "	<kerning first=\"344\" second=\"95\" amount=\"4\" />																																	"
	<< "	<kerning first=\"344\" second=\"67\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"343\" second=\"231\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"343\" second=\"42\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"343\" second=\"171\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"343\" second=\"34\" amount=\"2\" />																																	"
	<< "	<kerning first=\"343\" second=\"39\" amount=\"2\" />																																	"
	<< "	<kerning first=\"343\" second=\"44\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"343\" second=\"46\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"343\" second=\"95\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"343\" second=\"45\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"171\" second=\"84\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"171\" second=\"86\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"171\" second=\"354\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"171\" second=\"538\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"171\" second=\"356\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"171\" second=\"358\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"343\" second=\"47\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"343\" second=\"113\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"342\" second=\"268\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"342\" second=\"266\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"187\" second=\"84\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"187\" second=\"86\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"187\" second=\"354\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"187\" second=\"538\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"187\" second=\"356\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"187\" second=\"358\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"342\" second=\"264\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"342\" second=\"262\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"342\" second=\"199\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"342\" second=\"171\" amount=\"2\" />																																	"
	<< "	<kerning first=\"35\" second=\"52\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"42\" second=\"65\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"42\" second=\"67\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"42\" second=\"71\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"42\" second=\"79\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"42\" second=\"81\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"42\" second=\"83\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"42\" second=\"102\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"42\" second=\"114\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"42\" second=\"120\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"42\" second=\"192\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"42\" second=\"193\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"42\" second=\"194\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"42\" second=\"195\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"42\" second=\"196\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"42\" second=\"197\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"42\" second=\"199\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"42\" second=\"208\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"42\" second=\"210\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"42\" second=\"211\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"42\" second=\"212\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"42\" second=\"213\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"42\" second=\"214\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"42\" second=\"216\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"42\" second=\"338\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"42\" second=\"222\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"42\" second=\"352\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"42\" second=\"256\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"42\" second=\"258\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"42\" second=\"260\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"42\" second=\"506\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"42\" second=\"508\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"42\" second=\"262\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"42\" second=\"264\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"42\" second=\"266\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"42\" second=\"268\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"42\" second=\"270\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"42\" second=\"272\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"42\" second=\"284\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"42\" second=\"286\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"42\" second=\"288\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"42\" second=\"290\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"42\" second=\"332\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"42\" second=\"334\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"42\" second=\"336\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"42\" second=\"510\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"42\" second=\"346\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"42\" second=\"348\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"42\" second=\"350\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"42\" second=\"536\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"342\" second=\"63\" amount=\"5\" />																																	"
	<< "	<kerning first=\"342\" second=\"33\" amount=\"5\" />																																	"
	<< "	<kerning first=\"342\" second=\"59\" amount=\"4\" />																																	"
	<< "	<kerning first=\"342\" second=\"58\" amount=\"4\" />																																	"
	<< "	<kerning first=\"342\" second=\"44\" amount=\"5\" />																																	"
	<< "	<kerning first=\"342\" second=\"46\" amount=\"6\" />																																	"
	<< "	<kerning first=\"342\" second=\"95\" amount=\"4\" />																																	"
	<< "	<kerning first=\"342\" second=\"67\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"341\" second=\"231\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"341\" second=\"42\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"341\" second=\"171\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"341\" second=\"34\" amount=\"2\" />																																	"
	<< "	<kerning first=\"341\" second=\"39\" amount=\"2\" />																																	"
	<< "	<kerning first=\"341\" second=\"44\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"341\" second=\"46\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"341\" second=\"95\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"341\" second=\"45\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"341\" second=\"47\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"341\" second=\"113\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"340\" second=\"268\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"64\" second=\"67\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"64\" second=\"199\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"64\" second=\"260\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"64\" second=\"506\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"64\" second=\"262\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"64\" second=\"264\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"64\" second=\"266\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"64\" second=\"268\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"64\" second=\"333\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"64\" second=\"335\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"64\" second=\"337\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"340\" second=\"266\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"340\" second=\"264\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"340\" second=\"262\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"340\" second=\"199\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"340\" second=\"171\" amount=\"2\" />																																	"
	<< "	<kerning first=\"340\" second=\"63\" amount=\"5\" />																																	"
	<< "	<kerning first=\"340\" second=\"33\" amount=\"5\" />																																	"
	<< "	<kerning first=\"176\" second=\"67\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"176\" second=\"199\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"176\" second=\"262\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"176\" second=\"264\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"176\" second=\"266\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"176\" second=\"268\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"192\" second=\"84\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"192\" second=\"86\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"192\" second=\"47\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"192\" second=\"92\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"192\" second=\"39\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"192\" second=\"34\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"340\" second=\"59\" amount=\"4\" />																																	"
	<< "	<kerning first=\"340\" second=\"58\" amount=\"4\" />																																	"
	<< "	<kerning first=\"192\" second=\"35\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"192\" second=\"42\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"192\" second=\"174\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"340\" second=\"44\" amount=\"5\" />																																	"
	<< "	<kerning first=\"192\" second=\"354\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"192\" second=\"538\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"192\" second=\"356\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"192\" second=\"358\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"340\" second=\"46\" amount=\"6\" />																																	"
	<< "	<kerning first=\"340\" second=\"95\" amount=\"4\" />																																	"
	<< "	<kerning first=\"340\" second=\"67\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"511\" second=\"92\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"193\" second=\"84\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"193\" second=\"86\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"193\" second=\"47\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"193\" second=\"92\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"193\" second=\"39\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"193\" second=\"34\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"510\" second=\"44\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"510\" second=\"46\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"193\" second=\"35\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"193\" second=\"42\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"193\" second=\"174\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"510\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"193\" second=\"354\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"193\" second=\"538\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"193\" second=\"356\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"193\" second=\"358\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"510\" second=\"67\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"337\" second=\"92\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"336\" second=\"44\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"336\" second=\"46\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"194\" second=\"84\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"194\" second=\"86\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"194\" second=\"47\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"194\" second=\"92\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"194\" second=\"39\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"194\" second=\"34\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"336\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"336\" second=\"67\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"194\" second=\"35\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"194\" second=\"42\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"194\" second=\"174\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"335\" second=\"92\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"194\" second=\"354\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"194\" second=\"538\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"194\" second=\"356\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"194\" second=\"358\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"334\" second=\"44\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"334\" second=\"46\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"334\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"334\" second=\"67\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"195\" second=\"84\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"195\" second=\"86\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"195\" second=\"47\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"195\" second=\"92\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"195\" second=\"39\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"195\" second=\"34\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"333\" second=\"92\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"332\" second=\"44\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"195\" second=\"35\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"195\" second=\"42\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"195\" second=\"174\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"332\" second=\"46\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"195\" second=\"354\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"195\" second=\"538\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"195\" second=\"356\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"195\" second=\"358\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"332\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"332\" second=\"67\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"318\" second=\"318\" amount=\"3\" />																																	"
	<< "	<kerning first=\"318\" second=\"108\" amount=\"4\" />																																	"
	<< "	<kerning first=\"196\" second=\"84\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"196\" second=\"86\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"196\" second=\"47\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"196\" second=\"92\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"196\" second=\"39\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"196\" second=\"34\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"318\" second=\"107\" amount=\"2\" />																																	"
	<< "	<kerning first=\"318\" second=\"106\" amount=\"4\" />																																	"
	<< "	<kerning first=\"196\" second=\"35\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"196\" second=\"42\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"196\" second=\"174\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"318\" second=\"105\" amount=\"2\" />																																	"
	<< "	<kerning first=\"196\" second=\"354\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"196\" second=\"538\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"196\" second=\"356\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"196\" second=\"358\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"318\" second=\"104\" amount=\"2\" />																																	"
	<< "	<kerning first=\"318\" second=\"98\" amount=\"2\" />																																	"
	<< "	<kerning first=\"315\" second=\"372\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"315\" second=\"358\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"197\" second=\"84\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"197\" second=\"86\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"197\" second=\"47\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"197\" second=\"92\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"197\" second=\"39\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"197\" second=\"34\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"315\" second=\"356\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"315\" second=\"538\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"197\" second=\"35\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"197\" second=\"42\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"197\" second=\"174\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"315\" second=\"354\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"197\" second=\"354\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"197\" second=\"538\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"197\" second=\"356\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"197\" second=\"358\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"315\" second=\"174\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"315\" second=\"42\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"315\" second=\"34\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"315\" second=\"39\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"199\" second=\"80\" amount=\"2\" />																																	"
	<< "	<kerning first=\"199\" second=\"125\" amount=\"2\" />																																	"
	<< "	<kerning first=\"199\" second=\"124\" amount=\"2\" />																																	"
	<< "	<kerning first=\"199\" second=\"166\" amount=\"2\" />																																	"
	<< "	<kerning first=\"199\" second=\"92\" amount=\"2\" />																																	"
	<< "	<kerning first=\"199\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"315\" second=\"92\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"199\" second=\"63\" amount=\"2\" />																																	"
	<< "	<kerning first=\"199\" second=\"39\" amount=\"3\" />																																	"
	<< "	<kerning first=\"199\" second=\"34\" amount=\"3\" />																																	"
	<< "	<kerning first=\"199\" second=\"42\" amount=\"4\" />																																	"
	<< "	<kerning first=\"199\" second=\"64\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"199\" second=\"174\" amount=\"4\" />																																	"
	<< "	<kerning first=\"315\" second=\"87\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"199\" second=\"238\" amount=\"4\" />																																	"
	<< "	<kerning first=\"315\" second=\"86\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"315\" second=\"84\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"208\" second=\"67\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"208\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"313\" second=\"372\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"208\" second=\"46\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"208\" second=\"44\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"313\" second=\"358\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"313\" second=\"356\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"313\" second=\"538\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"321\" second=\"84\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"321\" second=\"87\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"321\" second=\"354\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"321\" second=\"538\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"321\" second=\"356\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"321\" second=\"358\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"321\" second=\"372\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"313\" second=\"354\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"313\" second=\"174\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"313\" second=\"42\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"313\" second=\"34\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"313\" second=\"39\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"313\" second=\"92\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"313\" second=\"87\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"210\" second=\"67\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"210\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"313\" second=\"86\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"210\" second=\"46\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"210\" second=\"44\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"313\" second=\"84\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"312\" second=\"511\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"337\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"211\" second=\"67\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"211\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"312\" second=\"335\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"211\" second=\"46\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"211\" second=\"44\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"312\" second=\"333\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"291\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"289\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"212\" second=\"67\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"212\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"312\" second=\"287\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"212\" second=\"46\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"212\" second=\"44\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"312\" second=\"285\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"283\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"281\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"213\" second=\"67\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"213\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"312\" second=\"279\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"213\" second=\"46\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"213\" second=\"44\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"312\" second=\"277\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"275\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"273\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"214\" second=\"67\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"214\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"312\" second=\"271\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"214\" second=\"46\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"214\" second=\"44\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"312\" second=\"269\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"265\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"263\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"216\" second=\"67\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"216\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"312\" second=\"509\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"216\" second=\"46\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"216\" second=\"44\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"312\" second=\"507\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"261\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"259\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"222\" second=\"67\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"222\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"312\" second=\"257\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"222\" second=\"46\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"222\" second=\"44\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"312\" second=\"339\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"248\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"246\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"352\" second=\"84\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"352\" second=\"86\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"352\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"312\" second=\"245\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"352\" second=\"46\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"352\" second=\"44\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"312\" second=\"244\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"243\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"242\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"240\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"235\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"352\" second=\"42\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"352\" second=\"174\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"312\" second=\"234\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"352\" second=\"354\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"352\" second=\"356\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"352\" second=\"358\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"312\" second=\"233\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"232\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"231\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"230\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"240\" second=\"92\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"242\" second=\"92\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"242\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"312\" second=\"229\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"243\" second=\"92\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"243\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"312\" second=\"228\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"244\" second=\"92\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"244\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"312\" second=\"227\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"245\" second=\"92\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"245\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"312\" second=\"226\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"246\" second=\"92\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"246\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"312\" second=\"225\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"248\" second=\"92\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"254\" second=\"92\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"223\" second=\"92\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"256\" second=\"84\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"256\" second=\"86\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"256\" second=\"47\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"256\" second=\"92\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"256\" second=\"39\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"256\" second=\"34\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"224\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"45\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"256\" second=\"35\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"256\" second=\"42\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"256\" second=\"174\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"312\" second=\"92\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"256\" second=\"354\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"256\" second=\"538\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"256\" second=\"356\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"256\" second=\"358\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"312\" second=\"120\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"113\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"111\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"103\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"258\" second=\"84\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"258\" second=\"86\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"258\" second=\"47\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"258\" second=\"92\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"258\" second=\"39\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"258\" second=\"34\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"101\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"100\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"258\" second=\"35\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"258\" second=\"42\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"258\" second=\"174\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"312\" second=\"99\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"258\" second=\"354\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"258\" second=\"538\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"258\" second=\"356\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"258\" second=\"358\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"312\" second=\"97\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"511\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"337\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"335\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"260\" second=\"84\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"260\" second=\"86\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"260\" second=\"47\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"260\" second=\"92\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"260\" second=\"39\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"260\" second=\"34\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"333\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"291\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"260\" second=\"35\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"260\" second=\"42\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"260\" second=\"174\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"311\" second=\"289\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"260\" second=\"354\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"260\" second=\"538\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"260\" second=\"356\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"260\" second=\"358\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"311\" second=\"287\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"285\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"283\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"281\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"506\" second=\"84\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"506\" second=\"86\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"506\" second=\"47\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"506\" second=\"92\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"506\" second=\"39\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"506\" second=\"34\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"279\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"277\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"506\" second=\"35\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"506\" second=\"42\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"506\" second=\"174\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"311\" second=\"275\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"506\" second=\"354\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"506\" second=\"538\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"506\" second=\"356\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"506\" second=\"358\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"311\" second=\"273\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"271\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"269\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"265\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"262\" second=\"80\" amount=\"2\" />																																	"
	<< "	<kerning first=\"262\" second=\"125\" amount=\"2\" />																																	"
	<< "	<kerning first=\"262\" second=\"124\" amount=\"2\" />																																	"
	<< "	<kerning first=\"262\" second=\"166\" amount=\"2\" />																																	"
	<< "	<kerning first=\"262\" second=\"92\" amount=\"2\" />																																	"
	<< "	<kerning first=\"262\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"311\" second=\"263\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"262\" second=\"63\" amount=\"2\" />																																	"
	<< "	<kerning first=\"262\" second=\"39\" amount=\"3\" />																																	"
	<< "	<kerning first=\"262\" second=\"34\" amount=\"3\" />																																	"
	<< "	<kerning first=\"262\" second=\"42\" amount=\"4\" />																																	"
	<< "	<kerning first=\"262\" second=\"64\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"262\" second=\"174\" amount=\"4\" />																																	"
	<< "	<kerning first=\"311\" second=\"509\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"262\" second=\"238\" amount=\"4\" />																																	"
	<< "	<kerning first=\"311\" second=\"507\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"261\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"264\" second=\"80\" amount=\"2\" />																																	"
	<< "	<kerning first=\"264\" second=\"125\" amount=\"2\" />																																	"
	<< "	<kerning first=\"264\" second=\"124\" amount=\"2\" />																																	"
	<< "	<kerning first=\"264\" second=\"166\" amount=\"2\" />																																	"
	<< "	<kerning first=\"264\" second=\"92\" amount=\"2\" />																																	"
	<< "	<kerning first=\"264\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"311\" second=\"259\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"264\" second=\"63\" amount=\"2\" />																																	"
	<< "	<kerning first=\"264\" second=\"39\" amount=\"3\" />																																	"
	<< "	<kerning first=\"264\" second=\"34\" amount=\"3\" />																																	"
	<< "	<kerning first=\"264\" second=\"42\" amount=\"4\" />																																	"
	<< "	<kerning first=\"264\" second=\"64\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"264\" second=\"174\" amount=\"4\" />																																	"
	<< "	<kerning first=\"311\" second=\"257\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"264\" second=\"238\" amount=\"4\" />																																	"
	<< "	<kerning first=\"311\" second=\"339\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"248\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"266\" second=\"125\" amount=\"2\" />																																	"
	<< "	<kerning first=\"266\" second=\"39\" amount=\"3\" />																																	"
	<< "	<kerning first=\"266\" second=\"34\" amount=\"3\" />																																	"
	<< "	<kerning first=\"266\" second=\"42\" amount=\"4\" />																																	"
	<< "	<kerning first=\"266\" second=\"64\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"266\" second=\"174\" amount=\"4\" />																																	"
	<< "	<kerning first=\"311\" second=\"246\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"266\" second=\"238\" amount=\"4\" />																																	"
	<< "	<kerning first=\"268\" second=\"80\" amount=\"2\" />																																	"
	<< "	<kerning first=\"268\" second=\"125\" amount=\"2\" />																																	"
	<< "	<kerning first=\"268\" second=\"124\" amount=\"2\" />																																	"
	<< "	<kerning first=\"268\" second=\"166\" amount=\"2\" />																																	"
	<< "	<kerning first=\"268\" second=\"92\" amount=\"2\" />																																	"
	<< "	<kerning first=\"268\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"311\" second=\"245\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"268\" second=\"63\" amount=\"2\" />																																	"
	<< "	<kerning first=\"268\" second=\"39\" amount=\"3\" />																																	"
	<< "	<kerning first=\"268\" second=\"34\" amount=\"3\" />																																	"
	<< "	<kerning first=\"268\" second=\"42\" amount=\"4\" />																																	"
	<< "	<kerning first=\"268\" second=\"64\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"268\" second=\"174\" amount=\"4\" />																																	"
	<< "	<kerning first=\"311\" second=\"244\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"268\" second=\"238\" amount=\"4\" />																																	"
	<< "	<kerning first=\"311\" second=\"243\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"242\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"270\" second=\"67\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"270\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"311\" second=\"240\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"270\" second=\"46\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"270\" second=\"44\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"311\" second=\"235\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"234\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"233\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"270\" second=\"42\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"271\" second=\"98\" amount=\"2\" />																																	"
	<< "	<kerning first=\"271\" second=\"104\" amount=\"2\" />																																	"
	<< "	<kerning first=\"271\" second=\"105\" amount=\"2\" />																																	"
	<< "	<kerning first=\"271\" second=\"106\" amount=\"4\" />																																	"
	<< "	<kerning first=\"271\" second=\"107\" amount=\"2\" />																																	"
	<< "	<kerning first=\"271\" second=\"108\" amount=\"4\" />																																	"
	<< "	<kerning first=\"271\" second=\"318\" amount=\"3\" />																																	"
	<< "	<kerning first=\"272\" second=\"67\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"272\" second=\"95\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"311\" second=\"232\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"272\" second=\"46\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"272\" second=\"44\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"311\" second=\"231\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"230\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"229\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"306\" second=\"46\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"306\" second=\"44\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"311\" second=\"228\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"227\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"226\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"308\" second=\"84\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"308\" second=\"99\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"308\" second=\"100\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"101\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"103\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"111\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"113\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"115\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"117\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"308\" second=\"119\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"308\" second=\"41\" amount=\"3\" />																																	"
	<< "	<kerning first=\"308\" second=\"93\" amount=\"3\" />																																	"
	<< "	<kerning first=\"308\" second=\"125\" amount=\"3\" />																																	"
	<< "	<kerning first=\"308\" second=\"46\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"44\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"311\" second=\"225\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"308\" second=\"33\" amount=\"2\" />																																	"
	<< "	<kerning first=\"308\" second=\"63\" amount=\"8\" />																																	"
	<< "	<kerning first=\"308\" second=\"39\" amount=\"9\" />																																	"
	<< "	<kerning first=\"308\" second=\"34\" amount=\"9\" />																																	"
	<< "	<kerning first=\"311\" second=\"224\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"45\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"311\" second=\"92\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"311\" second=\"120\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"308\" second=\"231\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"232\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"233\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"234\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"235\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"240\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"242\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"243\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"244\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"245\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"246\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"248\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"339\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"353\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"249\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"308\" second=\"250\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"308\" second=\"251\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"308\" second=\"252\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"308\" second=\"263\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"265\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"269\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"271\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"273\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"275\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"277\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"279\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"281\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"283\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"285\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"287\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"291\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"333\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"335\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"337\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"511\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"347\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"349\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"351\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"537\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"354\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"308\" second=\"538\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"308\" second=\"356\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"308\" second=\"358\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"308\" second=\"361\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"308\" second=\"363\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"308\" second=\"365\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"308\" second=\"367\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"308\" second=\"369\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"308\" second=\"371\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"308\" second=\"373\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"311\" second=\"113\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"111\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"103\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"101\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"100\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"99\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"97\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"511\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"510\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"337\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"336\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"335\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"334\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"333\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"332\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"291\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"290\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"287\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"286\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"285\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"284\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"283\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"281\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"279\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"277\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"275\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"273\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"272\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"67\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"71\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"79\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"81\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"99\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"100\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"101\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"103\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"106\" amount=\"2\" />																																	"
	<< "	<kerning first=\"310\" second=\"111\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"113\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"95\" amount=\"4\" />																																	"
	<< "	<kerning first=\"310\" second=\"271\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"46\" amount=\"6\" />																																	"
	<< "	<kerning first=\"310\" second=\"44\" amount=\"6\" />																																	"
	<< "	<kerning first=\"310\" second=\"58\" amount=\"6\" />																																	"
	<< "	<kerning first=\"310\" second=\"59\" amount=\"6\" />																																	"
	<< "	<kerning first=\"310\" second=\"270\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"33\" amount=\"6\" />																																	"
	<< "	<kerning first=\"310\" second=\"63\" amount=\"5\" />																																	"
	<< "	<kerning first=\"310\" second=\"269\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"268\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"265\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"264\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"171\" amount=\"5\" />																																	"
	<< "	<kerning first=\"310\" second=\"187\" amount=\"5\" />																																	"
	<< "	<kerning first=\"310\" second=\"208\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"210\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"211\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"212\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"213\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"214\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"216\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"338\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"222\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"231\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"232\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"233\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"234\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"235\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"240\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"242\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"243\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"244\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"245\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"246\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"248\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"339\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"262\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"263\" amount=\"-3\" />																																	"
	<< "	</kernings>																																												"
	<< "	</font>";
#pragma endregion Buxton_Xml

#pragma region Buxton_Nomal_Xml
	m_buildInFontXML[BUXTON_NORMAL]
	<< "	<?xml version=\"1.0\"?>																																									"
	<< "	<font>																																													"
	<< "	<info face=\"Buxton Sketch\" size=\"-64\" bold=\"0\" italic=\"0\" charset=\"\" unicode=\"1\" stretchH=\"100\" smooth=\"1\" aa=\"1\" padding=\"0,0,0,0\" spacing=\"1,1\" outline=\"0\"/>	"
	<< "	<common lineHeight=\"79\" base=\"54\" scaleW=\"1024\" scaleH=\"1024\" pages=\"1\" packed=\"0\" alphaChnl=\"1\" redChnl=\"0\" greenChnl=\"0\" blueChnl=\"0\"/>							"				\
	<< "	<pages>																																													"
	<< "	<page id=\"0\" file=\"buxton_nomal_0.png\" />																																			"
	<< "	</pages>																																												"
	<< "	<chars count=\"331\">																																									"
	<< "	<char id=\"32\" x=\"1017\" y=\"320\" width=\"3\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"15\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"33\" x=\"760\" y=\"640\" width=\"15\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"16\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"34\" x=\"650\" y=\"640\" width=\"18\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"20\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"35\" x=\"158\" y=\"0\" width=\"44\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"41\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"36\" x=\"440\" y=\"240\" width=\"30\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"33\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"37\" x=\"0\" y=\"160\" width=\"33\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"35\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"38\" x=\"0\" y=\"80\" width=\"39\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"41\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"39\" x=\"53\" y=\"720\" width=\"8\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"10\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"40\" x=\"917\" y=\"560\" width=\"23\" height=\"79\" xoffset=\"5\" yoffset=\"0\" xadvance=\"23\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"41\" x=\"514\" y=\"400\" width=\"27\" height=\"79\" xoffset=\"-8\" yoffset=\"0\" xadvance=\"23\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"42\" x=\"759\" y=\"80\" width=\"35\" height=\"79\" xoffset=\"-2\" yoffset=\"0\" xadvance=\"33\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"43\" x=\"540\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"44\" x=\"982\" y=\"640\" width=\"11\" height=\"79\" xoffset=\"3\" yoffset=\"0\" xadvance=\"16\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"45\" x=\"631\" y=\"640\" width=\"18\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"46\" x=\"0\" y=\"720\" width=\"10\" height=\"79\" xoffset=\"3\" yoffset=\"0\" xadvance=\"16\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"47\" x=\"287\" y=\"640\" width=\"20\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"17\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"48\" x=\"719\" y=\"240\" width=\"30\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"33\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"49\" x=\"994\" y=\"640\" width=\"11\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"16\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"50\" x=\"408\" y=\"160\" width=\"33\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"34\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"51\" x=\"975\" y=\"80\" width=\"35\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"37\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"52\" x=\"68\" y=\"160\" width=\"33\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"34\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"53\" x=\"843\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"54\" x=\"872\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"31\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"55\" x=\"136\" y=\"160\" width=\"33\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"56\" x=\"486\" y=\"400\" width=\"27\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"57\" x=\"564\" y=\"240\" width=\"30\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"58\" x=\"1006\" y=\"640\" width=\"11\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"16\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"59\" x=\"1011\" y=\"80\" width=\"12\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"16\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"60\" x=\"869\" y=\"560\" width=\"23\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"61\" x=\"324\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"62\" x=\"69\" y=\"640\" width=\"22\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"26\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"63\" x=\"621\" y=\"560\" width=\"24\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"64\" x=\"761\" y=\"0\" width=\"40\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"65\" x=\"866\" y=\"160\" width=\"31\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"66\" x=\"614\" y=\"80\" width=\"36\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"39\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"67\" x=\"871\" y=\"240\" width=\"29\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"68\" x=\"119\" y=\"80\" width=\"38\" height=\"79\" xoffset=\"-2\" yoffset=\"0\" xadvance=\"37\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"69\" x=\"297\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"70\" x=\"930\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"71\" x=\"204\" y=\"160\" width=\"33\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"72\" x=\"378\" y=\"240\" width=\"30\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"33\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"73\" x=\"43\" y=\"720\" width=\"9\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"13\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"74\" x=\"884\" y=\"0\" width=\"40\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"75\" x=\"248\" y=\"0\" width=\"44\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"38\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"76\" x=\"646\" y=\"560\" width=\"24\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"77\" x=\"40\" y=\"80\" width=\"39\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"41\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"78\" x=\"657\" y=\"240\" width=\"30\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"33\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"79\" x=\"988\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"80\" x=\"673\" y=\"160\" width=\"32\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"31\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"81\" x=\"293\" y=\"0\" width=\"43\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"41\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"82\" x=\"467\" y=\"0\" width=\"41\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"35\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"83\" x=\"651\" y=\"80\" width=\"35\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"84\" x=\"464\" y=\"80\" width=\"37\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"34\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"85\" x=\"0\" y=\"400\" width=\"28\" height=\"79\" xoffset=\"3\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"86\" x=\"939\" y=\"80\" width=\"35\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"31\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"87\" x=\"424\" y=\"0\" width=\"42\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"88\" x=\"285\" y=\"240\" width=\"30\" height=\"79\" xoffset=\"-2\" yoffset=\"0\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"89\" x=\"834\" y=\"160\" width=\"31\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"33\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"90\" x=\"640\" y=\"160\" width=\"32\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"91\" x=\"0\" y=\"560\" width=\"25\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"23\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"92\" x=\"821\" y=\"560\" width=\"23\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"17\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"93\" x=\"108\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"-4\" yoffset=\"0\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"94\" x=\"416\" y=\"560\" width=\"25\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"95\" x=\"781\" y=\"240\" width=\"29\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"96\" x=\"792\" y=\"640\" width=\"14\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"16\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"97\" x=\"29\" y=\"400\" width=\"28\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"24\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"98\" x=\"671\" y=\"560\" width=\"24\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"99\" x=\"988\" y=\"560\" width=\"22\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"23\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"100\" x=\"696\" y=\"560\" width=\"24\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"26\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"101\" x=\"887\" y=\"480\" width=\"25\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"102\" x=\"223\" y=\"240\" width=\"30\" height=\"79\" xoffset=\"-2\" yoffset=\"0\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"103\" x=\"27\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"26\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"104\" x=\"0\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"28\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"105\" x=\"934\" y=\"640\" width=\"11\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"106\" x=\"726\" y=\"640\" width=\"16\" height=\"79\" xoffset=\"-3\" yoffset=\"0\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"107\" x=\"346\" y=\"400\" width=\"27\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"108\" x=\"1011\" y=\"560\" width=\"12\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"109\" x=\"577\" y=\"80\" width=\"36\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"39\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"110\" x=\"861\" y=\"480\" width=\"25\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"111\" x=\"950\" y=\"400\" width=\"26\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"29\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"112\" x=\"174\" y=\"400\" width=\"28\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"113\" x=\"811\" y=\"240\" width=\"29\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"114\" x=\"203\" y=\"640\" width=\"20\" height=\"79\" xoffset=\"3\" yoffset=\"0\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"115\" x=\"261\" y=\"400\" width=\"28\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"116\" x=\"835\" y=\"480\" width=\"25\" height=\"79\" xoffset=\"-2\" yoffset=\"0\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"117\" x=\"896\" y=\"400\" width=\"26\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"118\" x=\"842\" y=\"400\" width=\"26\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"119\" x=\"426\" y=\"80\" width=\"37\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"38\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"120\" x=\"796\" y=\"560\" width=\"24\" height=\"79\" xoffset=\"-2\" yoffset=\"0\" xadvance=\"21\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"121\" x=\"809\" y=\"480\" width=\"25\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"122\" x=\"254\" y=\"240\" width=\"30\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"26\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"123\" x=\"893\" y=\"560\" width=\"23\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"124\" x=\"852\" y=\"640\" width=\"13\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"14\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"125\" x=\"138\" y=\"640\" width=\"21\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"126\" x=\"734\" y=\"400\" width=\"26\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"160\" x=\"1017\" y=\"480\" width=\"3\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"15\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"161\" x=\"11\" y=\"720\" width=\"10\" height=\"79\" xoffset=\"3\" yoffset=\"0\" xadvance=\"16\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"162\" x=\"494\" y=\"560\" width=\"25\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"163\" x=\"468\" y=\"560\" width=\"25\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"164\" x=\"442\" y=\"560\" width=\"25\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"26\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"165\" x=\"118\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"29\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"166\" x=\"946\" y=\"640\" width=\"11\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"14\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"167\" x=\"78\" y=\"560\" width=\"25\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"168\" x=\"707\" y=\"640\" width=\"18\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"169\" x=\"540\" y=\"80\" width=\"36\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"38\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"170\" x=\"392\" y=\"640\" width=\"20\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"171\" x=\"147\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"172\" x=\"26\" y=\"560\" width=\"25\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"173\" x=\"688\" y=\"640\" width=\"18\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"174\" x=\"723\" y=\"80\" width=\"35\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"175\" x=\"553\" y=\"640\" width=\"19\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"176\" x=\"350\" y=\"640\" width=\"20\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"20\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"177\" x=\"176\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"178\" x=\"965\" y=\"560\" width=\"22\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"23\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"179\" x=\"546\" y=\"560\" width=\"24\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"180\" x=\"807\" y=\"640\" width=\"14\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"16\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"181\" x=\"570\" y=\"400\" width=\"27\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"182\" x=\"541\" y=\"160\" width=\"32\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"35\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"183\" x=\"22\" y=\"720\" width=\"10\" height=\"79\" xoffset=\"3\" yoffset=\"0\" xadvance=\"16\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"184\" x=\"908\" y=\"640\" width=\"12\" height=\"79\" xoffset=\"3\" yoffset=\"0\" xadvance=\"18\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"185\" x=\"970\" y=\"640\" width=\"11\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"186\" x=\"329\" y=\"640\" width=\"20\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"187\" x=\"318\" y=\"400\" width=\"27\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"188\" x=\"719\" y=\"0\" width=\"41\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"189\" x=\"337\" y=\"0\" width=\"43\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"44\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"190\" x=\"0\" y=\"0\" width=\"53\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"53\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"191\" x=\"245\" y=\"640\" width=\"20\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"192\" x=\"96\" y=\"240\" width=\"31\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"193\" x=\"962\" y=\"160\" width=\"31\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"194\" x=\"930\" y=\"160\" width=\"31\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"195\" x=\"898\" y=\"160\" width=\"31\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"196\" x=\"802\" y=\"160\" width=\"31\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"197\" x=\"770\" y=\"160\" width=\"31\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"198\" x=\"802\" y=\"0\" width=\"40\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"38\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"199\" x=\"30\" y=\"320\" width=\"29\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"200\" x=\"405\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"201\" x=\"459\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"202\" x=\"513\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"203\" x=\"567\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"204\" x=\"822\" y=\"640\" width=\"14\" height=\"79\" xoffset=\"-3\" yoffset=\"0\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"205\" x=\"880\" y=\"640\" width=\"13\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"206\" x=\"493\" y=\"640\" width=\"19\" height=\"79\" xoffset=\"-3\" yoffset=\"0\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"207\" x=\"533\" y=\"640\" width=\"19\" height=\"79\" xoffset=\"-3\" yoffset=\"0\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"208\" x=\"551\" y=\"0\" width=\"41\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"42\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"209\" x=\"688\" y=\"240\" width=\"30\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"33\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"210\" x=\"263\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"211\" x=\"292\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"212\" x=\"321\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"213\" x=\"350\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"214\" x=\"379\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"215\" x=\"845\" y=\"560\" width=\"23\" height=\"79\" xoffset=\"3\" yoffset=\"0\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"216\" x=\"374\" y=\"160\" width=\"33\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"217\" x=\"234\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"3\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"218\" x=\"408\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"3\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"219\" x=\"437\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"3\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"220\" x=\"466\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"3\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"221\" x=\"64\" y=\"240\" width=\"31\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"33\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"222\" x=\"901\" y=\"240\" width=\"29\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"31\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"223\" x=\"750\" y=\"240\" width=\"30\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"33\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"224\" x=\"901\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"225\" x=\"959\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"226\" x=\"58\" y=\"400\" width=\"28\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"227\" x=\"116\" y=\"400\" width=\"28\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"228\" x=\"60\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"229\" x=\"205\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"230\" x=\"158\" y=\"80\" width=\"38\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"39\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"231\" x=\"92\" y=\"640\" width=\"22\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"23\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"232\" x=\"234\" y=\"560\" width=\"25\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"233\" x=\"364\" y=\"560\" width=\"25\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"234\" x=\"208\" y=\"560\" width=\"25\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"235\" x=\"130\" y=\"560\" width=\"25\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"236\" x=\"1006\" y=\"0\" width=\"17\" height=\"79\" xoffset=\"-5\" yoffset=\"0\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"237\" x=\"837\" y=\"640\" width=\"14\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"238\" x=\"612\" y=\"640\" width=\"18\" height=\"79\" xoffset=\"-3\" yoffset=\"0\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"239\" x=\"473\" y=\"640\" width=\"19\" height=\"79\" xoffset=\"-4\" yoffset=\"0\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"240\" x=\"841\" y=\"240\" width=\"29\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"241\" x=\"182\" y=\"560\" width=\"25\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"242\" x=\"756\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"29\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"243\" x=\"680\" y=\"400\" width=\"26\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"29\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"244\" x=\"788\" y=\"400\" width=\"26\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"29\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"245\" x=\"923\" y=\"400\" width=\"26\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"29\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"246\" x=\"54\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"29\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"247\" x=\"81\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"248\" x=\"458\" y=\"400\" width=\"27\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"249\" x=\"189\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"250\" x=\"243\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"251\" x=\"594\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"252\" x=\"729\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"253\" x=\"286\" y=\"560\" width=\"25\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"254\" x=\"626\" y=\"400\" width=\"26\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"255\" x=\"783\" y=\"480\" width=\"25\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"256\" x=\"32\" y=\"240\" width=\"31\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"257\" x=\"89\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"258\" x=\"0\" y=\"240\" width=\"31\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"259\" x=\"203\" y=\"400\" width=\"28\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"260\" x=\"607\" y=\"160\" width=\"32\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"261\" x=\"145\" y=\"400\" width=\"28\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"262\" x=\"931\" y=\"240\" width=\"29\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"263\" x=\"115\" y=\"640\" width=\"22\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"23\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"264\" x=\"0\" y=\"320\" width=\"29\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"30\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"265\" x=\"0\" y=\"640\" width=\"22\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"23\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"266\" x=\"994\" y=\"160\" width=\"29\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"267\" x=\"23\" y=\"640\" width=\"22\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"23\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"268\" x=\"991\" y=\"240\" width=\"29\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"269\" x=\"46\" y=\"640\" width=\"22\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"23\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"270\" x=\"80\" y=\"80\" width=\"38\" height=\"79\" xoffset=\"-2\" yoffset=\"0\" xadvance=\"37\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"271\" x=\"738\" y=\"160\" width=\"31\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"272\" x=\"677\" y=\"0\" width=\"41\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"42\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"273\" x=\"290\" y=\"400\" width=\"27\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"26\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"274\" x=\"351\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"275\" x=\"965\" y=\"480\" width=\"25\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"276\" x=\"378\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"277\" x=\"939\" y=\"480\" width=\"25\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"278\" x=\"486\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"279\" x=\"156\" y=\"560\" width=\"25\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"280\" x=\"702\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"281\" x=\"991\" y=\"480\" width=\"25\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"282\" x=\"977\" y=\"400\" width=\"26\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"283\" x=\"312\" y=\"560\" width=\"25\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"284\" x=\"170\" y=\"160\" width=\"33\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"285\" x=\"653\" y=\"400\" width=\"26\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"26\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"286\" x=\"238\" y=\"160\" width=\"33\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"287\" x=\"707\" y=\"400\" width=\"26\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"26\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"288\" x=\"272\" y=\"160\" width=\"33\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"289\" x=\"761\" y=\"400\" width=\"26\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"26\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"290\" x=\"306\" y=\"160\" width=\"33\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"291\" x=\"815\" y=\"400\" width=\"26\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"26\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"292\" x=\"626\" y=\"240\" width=\"30\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"33\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"293\" x=\"869\" y=\"400\" width=\"26\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"294\" x=\"312\" y=\"80\" width=\"37\" height=\"79\" xoffset=\"-3\" yoffset=\"0\" xadvance=\"33\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"295\" x=\"471\" y=\"240\" width=\"30\" height=\"79\" xoffset=\"-3\" yoffset=\"0\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"296\" x=\"371\" y=\"640\" width=\"20\" height=\"79\" xoffset=\"-4\" yoffset=\"0\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"297\" x=\"224\" y=\"640\" width=\"20\" height=\"79\" xoffset=\"-4\" yoffset=\"0\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"298\" x=\"433\" y=\"640\" width=\"19\" height=\"79\" xoffset=\"-3\" yoffset=\"0\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"299\" x=\"413\" y=\"640\" width=\"19\" height=\"79\" xoffset=\"-4\" yoffset=\"0\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"300\" x=\"182\" y=\"640\" width=\"20\" height=\"79\" xoffset=\"-4\" yoffset=\"0\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"301\" x=\"1004\" y=\"400\" width=\"19\" height=\"79\" xoffset=\"-4\" yoffset=\"0\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"302\" x=\"866\" y=\"640\" width=\"13\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"303\" x=\"776\" y=\"640\" width=\"15\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"304\" x=\"33\" y=\"720\" width=\"9\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"13\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"305\" x=\"958\" y=\"640\" width=\"11\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"306\" x=\"54\" y=\"0\" width=\"51\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"45\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"307\" x=\"571\" y=\"560\" width=\"24\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"26\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"308\" x=\"843\" y=\"0\" width=\"40\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"309\" x=\"669\" y=\"640\" width=\"18\" height=\"79\" xoffset=\"-5\" yoffset=\"0\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"310\" x=\"203\" y=\"0\" width=\"44\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"38\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"311\" x=\"374\" y=\"400\" width=\"27\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"312\" x=\"402\" y=\"400\" width=\"27\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"313\" x=\"596\" y=\"560\" width=\"24\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"314\" x=\"921\" y=\"640\" width=\"12\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"315\" x=\"721\" y=\"560\" width=\"24\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"316\" x=\"894\" y=\"640\" width=\"13\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"317\" x=\"746\" y=\"560\" width=\"24\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"318\" x=\"573\" y=\"640\" width=\"19\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"16\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"319\" x=\"771\" y=\"560\" width=\"24\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"25\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"320\" x=\"453\" y=\"640\" width=\"19\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"20\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"321\" x=\"442\" y=\"160\" width=\"32\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"322\" x=\"593\" y=\"640\" width=\"18\" height=\"79\" xoffset=\"-3\" yoffset=\"0\" xadvance=\"15\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"323\" x=\"409\" y=\"240\" width=\"30\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"33\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"324\" x=\"913\" y=\"480\" width=\"25\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"325\" x=\"316\" y=\"240\" width=\"30\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"33\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"326\" x=\"338\" y=\"560\" width=\"25\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"327\" x=\"347\" y=\"240\" width=\"30\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"33\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"328\" x=\"104\" y=\"560\" width=\"25\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"329\" x=\"340\" y=\"160\" width=\"33\" height=\"79\" xoffset=\"-6\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"330\" x=\"961\" y=\"240\" width=\"29\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"33\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"331\" x=\"941\" y=\"560\" width=\"23\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"332\" x=\"495\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"333\" x=\"162\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"29\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"334\" x=\"524\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"335\" x=\"216\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"29\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"336\" x=\"553\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"337\" x=\"270\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"29\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"338\" x=\"106\" y=\"0\" width=\"51\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"52\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"339\" x=\"966\" y=\"0\" width=\"39\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"40\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"340\" x=\"635\" y=\"0\" width=\"41\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"35\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"341\" x=\"308\" y=\"640\" width=\"20\" height=\"79\" xoffset=\"3\" yoffset=\"0\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"342\" x=\"593\" y=\"0\" width=\"41\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"35\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"343\" x=\"160\" y=\"640\" width=\"21\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"344\" x=\"509\" y=\"0\" width=\"41\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"35\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"345\" x=\"266\" y=\"640\" width=\"20\" height=\"79\" xoffset=\"3\" yoffset=\"0\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"346\" x=\"867\" y=\"80\" width=\"35\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"347\" x=\"582\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"348\" x=\"687\" y=\"80\" width=\"35\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"349\" x=\"611\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"350\" x=\"831\" y=\"80\" width=\"35\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"351\" x=\"640\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"352\" x=\"795\" y=\"80\" width=\"35\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"353\" x=\"669\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"354\" x=\"350\" y=\"80\" width=\"37\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"34\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"355\" x=\"390\" y=\"560\" width=\"25\" height=\"79\" xoffset=\"-2\" yoffset=\"0\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"356\" x=\"502\" y=\"80\" width=\"37\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"34\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"357\" x=\"34\" y=\"160\" width=\"33\" height=\"79\" xoffset=\"-2\" yoffset=\"0\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"358\" x=\"388\" y=\"80\" width=\"37\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"34\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"359\" x=\"260\" y=\"560\" width=\"25\" height=\"79\" xoffset=\"-2\" yoffset=\"0\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"360\" x=\"698\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"3\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"361\" x=\"432\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"362\" x=\"727\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"3\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"363\" x=\"135\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"364\" x=\"756\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"3\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"365\" x=\"621\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"366\" x=\"785\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"3\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"367\" x=\"648\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"368\" x=\"814\" y=\"320\" width=\"28\" height=\"79\" xoffset=\"3\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"369\" x=\"675\" y=\"480\" width=\"26\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"370\" x=\"192\" y=\"240\" width=\"30\" height=\"79\" xoffset=\"3\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"371\" x=\"598\" y=\"400\" width=\"27\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"372\" x=\"381\" y=\"0\" width=\"42\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"43\" page=\"0\" chnl=\"15\" />														"				\
	<< "	<char id=\"373\" x=\"236\" y=\"80\" width=\"37\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"38\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"374\" x=\"128\" y=\"240\" width=\"31\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"33\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"375\" x=\"52\" y=\"560\" width=\"25\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"376\" x=\"160\" y=\"240\" width=\"31\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"33\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"377\" x=\"574\" y=\"160\" width=\"32\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"378\" x=\"502\" y=\"240\" width=\"30\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"26\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"379\" x=\"475\" y=\"160\" width=\"32\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"380\" x=\"533\" y=\"240\" width=\"30\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"26\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"381\" x=\"508\" y=\"160\" width=\"32\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"32\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"382\" x=\"595\" y=\"240\" width=\"30\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"26\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"383\" x=\"513\" y=\"640\" width=\"19\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"15\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"402\" x=\"430\" y=\"400\" width=\"27\" height=\"79\" xoffset=\"-4\" yoffset=\"0\" xadvance=\"23\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"506\" x=\"706\" y=\"160\" width=\"31\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"27\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"507\" x=\"87\" y=\"400\" width=\"28\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"24\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"508\" x=\"925\" y=\"0\" width=\"40\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"38\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"509\" x=\"197\" y=\"80\" width=\"38\" height=\"79\" xoffset=\"1\" yoffset=\"0\" xadvance=\"39\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"510\" x=\"102\" y=\"160\" width=\"33\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"511\" x=\"542\" y=\"400\" width=\"27\" height=\"79\" xoffset=\"2\" yoffset=\"0\" xadvance=\"30\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"536\" x=\"903\" y=\"80\" width=\"35\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"36\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"537\" x=\"232\" y=\"400\" width=\"28\" height=\"79\" xoffset=\"0\" yoffset=\"0\" xadvance=\"28\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"538\" x=\"274\" y=\"80\" width=\"37\" height=\"79\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"34\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"539\" x=\"520\" y=\"560\" width=\"25\" height=\"79\" xoffset=\"-2\" yoffset=\"0\" xadvance=\"22\" page=\"0\" chnl=\"15\" />													"				\
	<< "	<char id=\"567\" x=\"743\" y=\"640\" width=\"16\" height=\"79\" xoffset=\"-3\" yoffset=\"0\" xadvance=\"13\" page=\"0\" chnl=\"15\" />													"				\
	<< "	</chars>																																												"
	<< "	<kernings count=\"2109\">																																								"
	<< "	<kerning first=\"65\" second=\"84\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"65\" second=\"86\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"65\" second=\"47\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"65\" second=\"92\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"65\" second=\"39\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"65\" second=\"34\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"373\" second=\"44\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"373\" second=\"46\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"65\" second=\"35\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"65\" second=\"42\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"65\" second=\"174\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"372\" second=\"358\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"65\" second=\"354\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"65\" second=\"538\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"65\" second=\"356\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"65\" second=\"358\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"372\" second=\"356\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"372\" second=\"538\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"372\" second=\"354\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"372\" second=\"44\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"66\" second=\"84\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"66\" second=\"86\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"66\" second=\"88\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"66\" second=\"93\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"66\" second=\"92\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"66\" second=\"95\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"372\" second=\"46\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"66\" second=\"46\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"66\" second=\"44\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"372\" second=\"95\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"66\" second=\"39\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"66\" second=\"34\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"372\" second=\"84\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"380\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"378\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"373\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"66\" second=\"35\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"66\" second=\"42\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"66\" second=\"174\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"372\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"66\" second=\"354\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"66\" second=\"538\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"66\" second=\"356\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"66\" second=\"358\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"371\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"369\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"367\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"365\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"363\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"361\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"359\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"67\" second=\"80\" amount=\"2\" />																																		"
	<< "	<kerning first=\"67\" second=\"125\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"67\" second=\"124\" amount=\"2\" />																																	"	\
	<< "	<kerning first=\"67\" second=\"166\" amount=\"2\" />																																	"	\
	<< "	<kerning first=\"67\" second=\"92\" amount=\"2\" />																																		"
	<< "	<kerning first=\"67\" second=\"95\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"357\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"67\" second=\"63\" amount=\"2\" />																																		"
	<< "	<kerning first=\"67\" second=\"39\" amount=\"3\" />																																		"
	<< "	<kerning first=\"67\" second=\"34\" amount=\"3\" />																																		"
	<< "	<kerning first=\"67\" second=\"42\" amount=\"4\" />																																		"
	<< "	<kerning first=\"67\" second=\"64\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"67\" second=\"174\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"539\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"67\" second=\"238\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"355\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"358\" second=\"537\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"68\" second=\"67\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"68\" second=\"95\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"536\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"68\" second=\"46\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"68\" second=\"44\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"351\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"358\" second=\"350\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"358\" second=\"349\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"68\" second=\"42\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"70\" second=\"124\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"70\" second=\"166\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"70\" second=\"92\" amount=\"3\" />																																		"
	<< "	<kerning first=\"70\" second=\"45\" amount=\"3\" />																																		"
	<< "	<kerning first=\"358\" second=\"348\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"358\" second=\"347\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"358\" second=\"346\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"70\" second=\"95\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"345\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"70\" second=\"46\" amount=\"-7\" />																																	"	\
	<< "	<kerning first=\"70\" second=\"44\" amount=\"-7\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"343\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"70\" second=\"63\" amount=\"2\" />																																		"
	<< "	<kerning first=\"70\" second=\"39\" amount=\"2\" />																																		"
	<< "	<kerning first=\"70\" second=\"34\" amount=\"3\" />																																		"
	<< "	<kerning first=\"358\" second=\"341\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"511\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"337\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"335\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"70\" second=\"42\" amount=\"3\" />																																		"
	<< "	<kerning first=\"70\" second=\"174\" amount=\"2\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"333\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"74\" second=\"84\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"99\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"100\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"101\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"103\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"111\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"113\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"115\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"117\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"119\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"41\" amount=\"3\" />																																		"
	<< "	<kerning first=\"74\" second=\"93\" amount=\"3\" />																																		"
	<< "	<kerning first=\"74\" second=\"125\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"46\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"44\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"331\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"74\" second=\"33\" amount=\"3\" />																																		"
	<< "	<kerning first=\"74\" second=\"63\" amount=\"9\" />																																		"
	<< "	<kerning first=\"74\" second=\"39\" amount=\"10\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"34\" amount=\"10\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"328\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"326\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"324\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"312\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"74\" second=\"231\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"232\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"233\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"234\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"235\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"240\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"242\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"243\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"244\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"245\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"246\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"248\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"339\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"353\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"249\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"250\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"251\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"252\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"263\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"265\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"269\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"271\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"273\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"275\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"277\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"279\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"281\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"283\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"285\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"287\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"291\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"333\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"335\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"337\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"511\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"347\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"349\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"351\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"537\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"354\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"538\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"356\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"358\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"361\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"363\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"365\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"367\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"369\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"371\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"74\" second=\"373\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"291\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"290\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"358\" second=\"289\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"288\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"358\" second=\"287\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"286\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"358\" second=\"285\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"284\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"358\" second=\"283\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"281\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"279\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"277\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"275\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"273\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"271\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"269\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"268\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"358\" second=\"266\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"358\" second=\"265\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"264\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"358\" second=\"263\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"262\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"358\" second=\"509\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"508\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"507\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"506\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"261\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"260\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"75\" second=\"67\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"71\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"79\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"81\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"84\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"99\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"100\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"101\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"103\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"106\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"111\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"113\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"95\" amount=\"4\" />																																		"
	<< "	<kerning first=\"358\" second=\"259\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"75\" second=\"46\" amount=\"6\" />																																		"
	<< "	<kerning first=\"75\" second=\"44\" amount=\"6\" />																																		"
	<< "	<kerning first=\"75\" second=\"58\" amount=\"6\" />																																		"
	<< "	<kerning first=\"75\" second=\"59\" amount=\"6\" />																																		"
	<< "	<kerning first=\"358\" second=\"258\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"257\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"75\" second=\"33\" amount=\"6\" />																																		"
	<< "	<kerning first=\"75\" second=\"63\" amount=\"6\" />																																		"
	<< "	<kerning first=\"358\" second=\"256\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"382\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"358\" second=\"253\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"252\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"358\" second=\"251\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"358\" second=\"250\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"75\" second=\"171\" amount=\"5\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"187\" amount=\"6\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"208\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"210\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"211\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"212\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"213\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"214\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"216\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"338\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"222\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"231\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"232\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"233\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"234\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"235\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"240\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"242\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"243\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"244\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"245\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"246\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"248\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"339\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"262\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"263\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"264\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"265\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"268\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"269\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"270\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"271\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"272\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"273\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"275\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"277\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"279\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"281\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"283\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"284\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"285\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"286\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"287\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"290\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"291\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"332\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"333\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"334\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"335\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"336\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"337\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"510\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"511\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"354\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"356\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"75\" second=\"358\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"249\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"358\" second=\"353\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"358\" second=\"339\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"248\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"246\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"245\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"244\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"243\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"242\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"241\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"305\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"240\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"235\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"234\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"233\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"232\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"358\" second=\"231\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"230\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"229\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"228\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"227\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"226\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"358\" second=\"225\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"224\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"358\" second=\"352\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"358\" second=\"321\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"358\" second=\"199\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"358\" second=\"197\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"196\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"195\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"76\" second=\"84\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"76\" second=\"86\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"76\" second=\"87\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"76\" second=\"92\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"76\" second=\"39\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"76\" second=\"34\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"194\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"358\" second=\"193\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"76\" second=\"42\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"76\" second=\"174\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"192\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"76\" second=\"354\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"76\" second=\"538\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"76\" second=\"356\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"76\" second=\"358\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"76\" second=\"372\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"174\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"64\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"42\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"187\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"358\" second=\"171\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"358\" second=\"34\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"39\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"79\" second=\"67\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"79\" second=\"95\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"59\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"79\" second=\"46\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"79\" second=\"44\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"58\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"44\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"46\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"65\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"67\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"38\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"97\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"99\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"100\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"101\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"103\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"111\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"113\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"115\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"47\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"45\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"95\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"45\" amount=\"-9\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"47\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"95\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"122\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"80\" second=\"46\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"44\" amount=\"-14\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"121\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"80\" second=\"63\" amount=\"3\" />																																		"
	<< "	<kerning first=\"358\" second=\"120\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"119\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"118\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"80\" second=\"171\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"42\" amount=\"3\" />																																		"
	<< "	<kerning first=\"80\" second=\"64\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"192\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"193\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"194\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"195\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"196\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"197\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"199\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"321\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"224\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"225\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"226\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"227\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"228\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"229\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"230\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"231\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"232\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"233\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"234\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"235\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"240\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"242\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"243\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"244\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"245\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"246\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"248\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"339\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"353\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"256\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"257\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"258\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"259\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"260\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"261\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"506\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"507\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"508\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"509\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"262\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"263\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"264\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"265\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"266\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"268\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"269\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"271\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"273\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"275\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"277\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"279\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"281\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"283\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"285\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"287\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"289\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"291\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"333\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"335\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"337\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"511\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"347\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"349\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"351\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"80\" second=\"537\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"117\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"116\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"358\" second=\"115\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"358\" second=\"114\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"113\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"112\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"111\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"110\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"109\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"106\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"358\" second=\"105\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"358\" second=\"104\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"358\" second=\"103\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"102\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"358\" second=\"101\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"100\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"358\" second=\"99\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"97\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"81\" second=\"67\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"81\" second=\"95\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"88\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"81\" second=\"46\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"81\" second=\"44\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"87\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"86\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"358\" second=\"83\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"82\" second=\"67\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"82\" second=\"95\" amount=\"4\" />																																		"
	<< "	<kerning first=\"358\" second=\"71\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"82\" second=\"46\" amount=\"6\" />																																		"
	<< "	<kerning first=\"82\" second=\"44\" amount=\"5\" />																																		"
	<< "	<kerning first=\"82\" second=\"58\" amount=\"4\" />																																		"
	<< "	<kerning first=\"82\" second=\"59\" amount=\"4\" />																																		"
	<< "	<kerning first=\"358\" second=\"67\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"82\" second=\"33\" amount=\"6\" />																																		"
	<< "	<kerning first=\"82\" second=\"63\" amount=\"6\" />																																		"
	<< "	<kerning first=\"358\" second=\"65\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"357\" second=\"318\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"357\" second=\"108\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"82\" second=\"171\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"82\" second=\"199\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"82\" second=\"262\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"82\" second=\"264\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"82\" second=\"266\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"82\" second=\"268\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"83\" second=\"84\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"83\" second=\"95\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"357\" second=\"107\" amount=\"2\" />																																	"	\
	<< "	<kerning first=\"83\" second=\"46\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"83\" second=\"44\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"357\" second=\"106\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"357\" second=\"105\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"357\" second=\"104\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"357\" second=\"98\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"380\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"83\" second=\"42\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"83\" second=\"354\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"83\" second=\"356\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"83\" second=\"358\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"378\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"373\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"372\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"371\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"65\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"67\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"71\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"83\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"86\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"87\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"88\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"97\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"99\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"100\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"101\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"102\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"103\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"104\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"105\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"106\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"109\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"110\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"111\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"112\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"113\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"114\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"115\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"116\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"117\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"118\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"119\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"120\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"121\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"122\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"47\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"45\" amount=\"-9\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"369\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"367\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"365\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"95\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"363\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"46\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"44\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"58\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"59\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"361\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"84\" second=\"39\" amount=\"4\" />																																		"
	<< "	<kerning first=\"84\" second=\"34\" amount=\"4\" />																																		"
	<< "	<kerning first=\"356\" second=\"359\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"356\" second=\"357\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"356\" second=\"539\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"356\" second=\"355\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"84\" second=\"171\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"187\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"42\" amount=\"4\" />																																		"
	<< "	<kerning first=\"84\" second=\"64\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"174\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"537\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"84\" second=\"192\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"193\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"194\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"195\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"196\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"197\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"199\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"321\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"352\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"224\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"225\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"226\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"227\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"228\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"229\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"230\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"231\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"232\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"233\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"234\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"235\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"240\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"305\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"241\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"242\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"243\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"244\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"245\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"246\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"248\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"339\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"353\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"249\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"250\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"251\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"252\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"253\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"382\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"256\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"257\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"258\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"259\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"260\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"261\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"506\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"507\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"508\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"509\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"262\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"263\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"264\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"265\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"266\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"268\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"269\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"271\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"273\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"275\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"277\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"279\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"281\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"283\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"284\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"285\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"286\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"287\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"288\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"289\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"290\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"291\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"312\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"324\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"326\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"328\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"331\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"333\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"335\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"337\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"511\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"341\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"343\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"345\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"346\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"347\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"348\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"349\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"350\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"351\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"536\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"537\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"355\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"539\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"357\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"359\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"361\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"363\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"365\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"367\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"369\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"371\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"372\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"373\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"536\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"356\" second=\"351\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"356\" second=\"350\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"356\" second=\"349\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"356\" second=\"348\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"356\" second=\"347\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"84\" second=\"378\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"84\" second=\"380\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"346\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"356\" second=\"345\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"343\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"341\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"511\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"337\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"335\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"333\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"331\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"328\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"326\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"324\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"312\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"291\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"290\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"289\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"288\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"287\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"286\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"285\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"284\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"283\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"281\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"279\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"277\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"275\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"273\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"271\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"269\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"268\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"266\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"265\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"264\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"263\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"262\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"509\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"508\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"507\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"506\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"261\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"260\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"259\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"258\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"257\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"256\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"382\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"253\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"252\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"251\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"250\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"249\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"353\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"356\" second=\"339\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"248\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"246\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"245\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"244\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"86\" second=\"67\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"80\" amount=\"3\" />																																		"
	<< "	<kerning first=\"86\" second=\"84\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"97\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"99\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"100\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"101\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"103\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"111\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"113\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"115\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"47\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"45\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"243\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"242\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"241\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"86\" second=\"95\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"305\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"86\" second=\"46\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"44\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"240\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"86\" second=\"63\" amount=\"3\" />																																		"
	<< "	<kerning first=\"86\" second=\"39\" amount=\"6\" />																																		"
	<< "	<kerning first=\"86\" second=\"34\" amount=\"6\" />																																		"
	<< "	<kerning first=\"356\" second=\"235\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"234\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"233\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"232\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"356\" second=\"231\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"86\" second=\"171\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"42\" amount=\"6\" />																																		"
	<< "	<kerning first=\"86\" second=\"64\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"174\" amount=\"6\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"230\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"86\" second=\"224\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"225\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"226\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"227\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"228\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"229\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"230\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"231\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"232\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"233\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"234\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"235\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"240\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"242\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"243\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"244\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"245\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"246\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"248\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"339\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"353\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"257\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"259\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"261\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"507\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"509\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"263\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"265\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"269\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"271\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"273\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"275\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"277\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"279\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"281\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"283\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"285\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"287\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"289\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"291\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"333\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"335\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"337\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"511\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"347\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"349\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"351\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"537\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"354\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"538\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"356\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"86\" second=\"358\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"229\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"228\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"227\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"226\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"225\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"224\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"356\" second=\"352\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"356\" second=\"321\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"356\" second=\"199\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"197\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"196\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"195\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"194\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"193\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"192\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"356\" second=\"174\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"64\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"42\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"187\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"171\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"356\" second=\"34\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"39\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"59\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"87\" second=\"84\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"87\" second=\"95\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"58\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"87\" second=\"46\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"87\" second=\"44\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"44\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"46\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"95\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"45\" amount=\"-9\" />																																	"	\
	<< "	<kerning first=\"87\" second=\"354\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"87\" second=\"538\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"87\" second=\"356\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"87\" second=\"358\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"47\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"122\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"121\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"120\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"88\" second=\"84\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"119\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"118\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"88\" second=\"354\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"88\" second=\"538\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"88\" second=\"356\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"88\" second=\"358\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"117\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"116\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"356\" second=\"115\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"356\" second=\"114\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"98\" second=\"92\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"102\" second=\"102\" amount=\"-7\" />																																	"
	<< "	<kerning first=\"102\" second=\"41\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"102\" second=\"93\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"102\" second=\"125\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"102\" second=\"124\" amount=\"6\" />																																	"	\
	<< "	<kerning first=\"102\" second=\"166\" amount=\"6\" />																																	"	\
	<< "	<kerning first=\"102\" second=\"39\" amount=\"6\" />																																	"	\
	<< "	<kerning first=\"102\" second=\"34\" amount=\"6\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"113\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"112\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"102\" second=\"42\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"102\" second=\"174\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"111\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"107\" second=\"97\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"107\" second=\"99\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"107\" second=\"100\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"101\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"103\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"111\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"113\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"120\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"92\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"107\" second=\"45\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"110\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"109\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"106\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"105\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"104\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"224\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"225\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"226\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"227\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"228\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"229\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"230\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"231\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"232\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"233\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"234\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"235\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"240\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"242\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"243\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"244\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"245\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"246\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"248\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"339\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"257\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"259\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"261\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"507\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"509\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"263\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"265\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"269\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"271\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"273\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"275\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"277\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"279\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"281\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"283\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"285\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"287\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"289\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"291\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"333\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"335\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"337\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"107\" second=\"511\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"356\" second=\"103\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"102\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"356\" second=\"101\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"100\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"356\" second=\"99\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"97\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"88\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"87\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"86\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"83\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"71\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"67\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"356\" second=\"65\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"380\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"378\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"373\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"372\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"538\" second=\"371\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"111\" second=\"92\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"112\" second=\"92\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"112\" second=\"95\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"369\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"112\" second=\"46\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"112\" second=\"44\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"367\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"365\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"363\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"114\" second=\"113\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"114\" second=\"47\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"114\" second=\"45\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"361\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"359\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"538\" second=\"357\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"114\" second=\"95\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"539\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"114\" second=\"46\" amount=\"-9\" />																																	"	\
	<< "	<kerning first=\"114\" second=\"44\" amount=\"-9\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"355\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"114\" second=\"39\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"114\" second=\"34\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"537\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"538\" second=\"351\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"538\" second=\"349\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"114\" second=\"171\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"114\" second=\"42\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"114\" second=\"231\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"118\" second=\"95\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"347\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"118\" second=\"46\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"118\" second=\"44\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"345\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"118\" second=\"63\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"118\" second=\"39\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"118\" second=\"34\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"343\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"341\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"119\" second=\"46\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"119\" second=\"44\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"511\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"337\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"335\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"120\" second=\"39\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"120\" second=\"34\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"67\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"99\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"100\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"101\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"103\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"111\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"113\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"199\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"231\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"232\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"233\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"234\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"235\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"240\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"242\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"243\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"244\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"245\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"246\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"248\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"339\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"262\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"263\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"264\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"265\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"266\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"268\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"269\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"271\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"273\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"275\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"277\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"279\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"281\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"283\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"285\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"287\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"291\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"333\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"335\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"337\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"47\" second=\"511\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"333\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"331\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"328\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"326\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"324\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"312\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"291\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"290\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"538\" second=\"289\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"288\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"538\" second=\"287\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"286\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"538\" second=\"285\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"284\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"538\" second=\"283\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"92\" second=\"84\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"92\" second=\"86\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"92\" second=\"87\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"92\" second=\"113\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"92\" second=\"354\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"92\" second=\"538\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"92\" second=\"356\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"92\" second=\"358\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"92\" second=\"372\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"281\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"279\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"277\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"275\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"273\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"271\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"269\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"45\" second=\"84\" amount=\"-9\" />																																	"	\
	<< "	<kerning first=\"45\" second=\"86\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"45\" second=\"88\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"45\" second=\"354\" amount=\"-9\" />																																	"	\
	<< "	<kerning first=\"45\" second=\"538\" amount=\"-9\" />																																	"	\
	<< "	<kerning first=\"45\" second=\"356\" amount=\"-9\" />																																	"	\
	<< "	<kerning first=\"45\" second=\"358\" amount=\"-9\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"268\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"538\" second=\"266\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"538\" second=\"265\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"264\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"538\" second=\"263\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"262\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"538\" second=\"509\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"507\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"261\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"259\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"257\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"382\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"538\" second=\"253\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"252\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"538\" second=\"251\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"538\" second=\"250\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"249\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"538\" second=\"353\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"538\" second=\"339\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"248\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"246\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"245\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"244\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"243\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"242\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"241\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"305\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"240\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"235\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"234\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"233\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"232\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"538\" second=\"231\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"230\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"229\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"228\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"227\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"226\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"538\" second=\"225\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"224\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"538\" second=\"321\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"538\" second=\"199\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"538\" second=\"174\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"64\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"42\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"187\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"538\" second=\"171\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"538\" second=\"34\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"39\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"95\" second=\"84\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"95\" second=\"118\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"95\" second=\"120\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"95\" second=\"354\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"95\" second=\"538\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"95\" second=\"356\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"95\" second=\"358\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"59\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"58\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"44\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"46\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"95\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"45\" amount=\"-9\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"47\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"122\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"121\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"120\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"119\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"118\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"117\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"116\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"538\" second=\"115\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"538\" second=\"114\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"113\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"112\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"111\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"110\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"109\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"106\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"538\" second=\"105\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"538\" second=\"104\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"538\" second=\"103\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"46\" second=\"84\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"46\" second=\"86\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"46\" second=\"87\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"46\" second=\"354\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"46\" second=\"538\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"46\" second=\"356\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"46\" second=\"358\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"46\" second=\"372\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"102\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"538\" second=\"101\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"100\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"538\" second=\"99\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"97\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"88\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"87\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"86\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"71\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"538\" second=\"67\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"354\" second=\"380\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"378\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"373\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"372\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"354\" second=\"371\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"369\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"367\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"365\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"363\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"361\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"191\" second=\"88\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"354\" second=\"359\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"354\" second=\"357\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"354\" second=\"539\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"39\" second=\"67\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"39\" second=\"102\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"39\" second=\"113\" amount=\"-7\" />																																	"	\
	<< "	<kerning first=\"39\" second=\"199\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"39\" second=\"262\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"39\" second=\"264\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"39\" second=\"266\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"39\" second=\"268\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"34\" second=\"65\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"34\" second=\"67\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"34\" second=\"102\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"34\" second=\"113\" amount=\"-7\" />																																	"	\
	<< "	<kerning first=\"34\" second=\"120\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"34\" second=\"192\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"34\" second=\"193\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"34\" second=\"194\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"34\" second=\"195\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"34\" second=\"196\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"34\" second=\"197\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"34\" second=\"199\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"34\" second=\"256\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"34\" second=\"258\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"34\" second=\"260\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"34\" second=\"506\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"34\" second=\"508\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"34\" second=\"262\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"34\" second=\"264\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"34\" second=\"266\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"34\" second=\"268\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"354\" second=\"355\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"354\" second=\"537\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"354\" second=\"536\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"354\" second=\"351\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"354\" second=\"350\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"354\" second=\"349\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"354\" second=\"348\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"354\" second=\"347\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"354\" second=\"346\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"354\" second=\"345\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"343\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"341\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"511\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"337\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"335\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"333\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"331\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"328\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"326\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"324\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"312\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"291\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"290\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"354\" second=\"289\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"288\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"354\" second=\"287\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"286\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"354\" second=\"285\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"284\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"354\" second=\"283\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"281\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"279\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"277\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"275\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"273\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"271\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"269\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"268\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"354\" second=\"266\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"354\" second=\"265\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"264\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"354\" second=\"263\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"262\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"354\" second=\"509\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"508\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"507\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"506\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"261\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"260\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"259\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"258\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"257\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"256\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"382\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"354\" second=\"253\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"252\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"354\" second=\"251\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"354\" second=\"250\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"249\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"354\" second=\"353\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"354\" second=\"339\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"248\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"246\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"245\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"244\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"243\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"242\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"241\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"305\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"240\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"235\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"234\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"233\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"232\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"354\" second=\"231\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"230\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"229\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"228\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"227\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"226\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"354\" second=\"225\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"224\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"354\" second=\"352\" amount=\"-1\" />																																	"
	<< "	<kerning first=\"354\" second=\"321\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"354\" second=\"199\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"354\" second=\"197\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"196\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"195\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"194\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"193\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"192\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"354\" second=\"174\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"354\" second=\"64\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"354\" second=\"42\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"354\" second=\"187\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"354\" second=\"171\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"354\" second=\"34\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"354\" second=\"39\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"354\" second=\"59\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"354\" second=\"58\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"354\" second=\"44\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"354\" second=\"46\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"354\" second=\"95\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"354\" second=\"45\" amount=\"-9\" />																																	"	\
	<< "	<kerning first=\"354\" second=\"47\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"354\" second=\"122\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"121\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"120\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"119\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"118\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"117\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"116\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"354\" second=\"115\" amount=\"-8\" />																																	"
	<< "	<kerning first=\"354\" second=\"114\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"113\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"112\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"111\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"110\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"109\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"106\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"354\" second=\"105\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"354\" second=\"104\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"354\" second=\"103\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"102\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"354\" second=\"101\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"100\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"354\" second=\"99\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"354\" second=\"97\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"354\" second=\"88\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"354\" second=\"87\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"354\" second=\"86\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"354\" second=\"83\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"354\" second=\"71\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"354\" second=\"67\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"354\" second=\"65\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"383\" second=\"44\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"383\" second=\"46\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"536\" second=\"358\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"536\" second=\"356\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"536\" second=\"354\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"536\" second=\"42\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"536\" second=\"44\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"536\" second=\"46\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"536\" second=\"95\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"536\" second=\"84\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"350\" second=\"358\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"350\" second=\"356\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"350\" second=\"354\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"350\" second=\"42\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"350\" second=\"44\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"350\" second=\"46\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"350\" second=\"95\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"350\" second=\"84\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"348\" second=\"358\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"348\" second=\"356\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"348\" second=\"354\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"348\" second=\"42\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"348\" second=\"44\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"348\" second=\"46\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"348\" second=\"95\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"348\" second=\"84\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"346\" second=\"358\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"346\" second=\"356\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"346\" second=\"354\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"346\" second=\"42\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"346\" second=\"44\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"346\" second=\"46\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"346\" second=\"95\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"346\" second=\"84\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"345\" second=\"231\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"345\" second=\"42\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"345\" second=\"171\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"345\" second=\"34\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"345\" second=\"39\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"345\" second=\"44\" amount=\"-9\" />																																	"	\
	<< "	<kerning first=\"345\" second=\"46\" amount=\"-9\" />																																	"	\
	<< "	<kerning first=\"345\" second=\"95\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"345\" second=\"45\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"345\" second=\"47\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"345\" second=\"113\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"344\" second=\"268\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"344\" second=\"266\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"344\" second=\"264\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"344\" second=\"262\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"344\" second=\"199\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"344\" second=\"171\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"344\" second=\"63\" amount=\"6\" />																																	"	\
	<< "	<kerning first=\"344\" second=\"33\" amount=\"6\" />																																	"	\
	<< "	<kerning first=\"344\" second=\"59\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"344\" second=\"58\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"344\" second=\"44\" amount=\"5\" />																																	"	\
	<< "	<kerning first=\"344\" second=\"46\" amount=\"6\" />																																	"	\
	<< "	<kerning first=\"344\" second=\"95\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"344\" second=\"67\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"343\" second=\"231\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"343\" second=\"42\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"343\" second=\"171\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"343\" second=\"34\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"343\" second=\"39\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"343\" second=\"44\" amount=\"-9\" />																																	"	\
	<< "	<kerning first=\"343\" second=\"46\" amount=\"-9\" />																																	"	\
	<< "	<kerning first=\"343\" second=\"95\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"343\" second=\"45\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"171\" second=\"84\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"171\" second=\"86\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"171\" second=\"354\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"171\" second=\"538\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"171\" second=\"356\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"171\" second=\"358\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"343\" second=\"47\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"343\" second=\"113\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"342\" second=\"268\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"342\" second=\"266\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"187\" second=\"84\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"187\" second=\"86\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"187\" second=\"354\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"187\" second=\"538\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"187\" second=\"356\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"187\" second=\"358\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"342\" second=\"264\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"342\" second=\"262\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"342\" second=\"199\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"342\" second=\"171\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"35\" second=\"52\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"65\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"67\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"71\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"79\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"81\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"83\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"102\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"114\" amount=\"-7\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"120\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"192\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"193\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"194\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"195\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"196\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"197\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"199\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"208\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"210\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"211\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"212\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"213\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"214\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"216\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"338\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"222\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"352\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"256\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"258\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"260\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"506\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"508\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"262\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"264\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"266\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"268\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"270\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"272\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"284\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"286\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"288\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"290\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"332\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"334\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"336\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"510\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"346\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"348\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"350\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"42\" second=\"536\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"342\" second=\"63\" amount=\"6\" />																																	"	\
	<< "	<kerning first=\"342\" second=\"33\" amount=\"6\" />																																	"	\
	<< "	<kerning first=\"342\" second=\"59\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"342\" second=\"58\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"342\" second=\"44\" amount=\"5\" />																																	"	\
	<< "	<kerning first=\"342\" second=\"46\" amount=\"6\" />																																	"	\
	<< "	<kerning first=\"342\" second=\"95\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"342\" second=\"67\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"341\" second=\"231\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"341\" second=\"42\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"341\" second=\"171\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"341\" second=\"34\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"341\" second=\"39\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"341\" second=\"44\" amount=\"-9\" />																																	"	\
	<< "	<kerning first=\"341\" second=\"46\" amount=\"-9\" />																																	"	\
	<< "	<kerning first=\"341\" second=\"95\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"341\" second=\"45\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"341\" second=\"47\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"341\" second=\"113\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"340\" second=\"268\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"64\" second=\"67\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"64\" second=\"199\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"64\" second=\"260\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"64\" second=\"506\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"64\" second=\"262\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"64\" second=\"264\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"64\" second=\"266\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"64\" second=\"268\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"64\" second=\"333\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"64\" second=\"335\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"64\" second=\"337\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"340\" second=\"266\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"340\" second=\"264\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"340\" second=\"262\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"340\" second=\"199\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"340\" second=\"171\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"340\" second=\"63\" amount=\"6\" />																																	"	\
	<< "	<kerning first=\"340\" second=\"33\" amount=\"6\" />																																	"	\
	<< "	<kerning first=\"176\" second=\"67\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"176\" second=\"199\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"176\" second=\"262\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"176\" second=\"264\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"176\" second=\"266\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"176\" second=\"268\" amount=\"-6\" />																																	"
	<< "	<kerning first=\"192\" second=\"84\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"192\" second=\"86\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"192\" second=\"47\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"192\" second=\"92\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"192\" second=\"39\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"192\" second=\"34\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"340\" second=\"59\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"340\" second=\"58\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"192\" second=\"35\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"192\" second=\"42\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"192\" second=\"174\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"340\" second=\"44\" amount=\"5\" />																																	"	\
	<< "	<kerning first=\"192\" second=\"354\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"192\" second=\"538\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"192\" second=\"356\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"192\" second=\"358\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"340\" second=\"46\" amount=\"6\" />																																	"	\
	<< "	<kerning first=\"340\" second=\"95\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"340\" second=\"67\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"511\" second=\"92\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"193\" second=\"84\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"193\" second=\"86\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"193\" second=\"47\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"193\" second=\"92\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"193\" second=\"39\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"193\" second=\"34\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"510\" second=\"44\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"510\" second=\"46\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"193\" second=\"35\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"193\" second=\"42\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"193\" second=\"174\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"510\" second=\"95\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"193\" second=\"354\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"193\" second=\"538\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"193\" second=\"356\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"193\" second=\"358\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"510\" second=\"67\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"337\" second=\"92\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"336\" second=\"44\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"336\" second=\"46\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"194\" second=\"84\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"194\" second=\"86\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"194\" second=\"47\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"194\" second=\"92\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"194\" second=\"39\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"194\" second=\"34\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"336\" second=\"95\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"336\" second=\"67\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"194\" second=\"35\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"194\" second=\"42\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"194\" second=\"174\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"335\" second=\"92\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"194\" second=\"354\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"194\" second=\"538\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"194\" second=\"356\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"194\" second=\"358\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"334\" second=\"44\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"334\" second=\"46\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"334\" second=\"95\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"334\" second=\"67\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"195\" second=\"84\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"195\" second=\"86\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"195\" second=\"47\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"195\" second=\"92\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"195\" second=\"39\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"195\" second=\"34\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"333\" second=\"92\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"332\" second=\"44\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"195\" second=\"35\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"195\" second=\"42\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"195\" second=\"174\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"332\" second=\"46\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"195\" second=\"354\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"195\" second=\"538\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"195\" second=\"356\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"195\" second=\"358\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"332\" second=\"95\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"332\" second=\"67\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"318\" second=\"318\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"318\" second=\"108\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"196\" second=\"84\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"196\" second=\"86\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"196\" second=\"47\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"196\" second=\"92\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"196\" second=\"39\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"196\" second=\"34\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"318\" second=\"107\" amount=\"2\" />																																	"	\
	<< "	<kerning first=\"318\" second=\"106\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"196\" second=\"35\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"196\" second=\"42\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"196\" second=\"174\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"318\" second=\"105\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"196\" second=\"354\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"196\" second=\"538\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"196\" second=\"356\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"196\" second=\"358\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"318\" second=\"104\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"318\" second=\"98\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"315\" second=\"372\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"315\" second=\"358\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"197\" second=\"84\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"197\" second=\"86\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"197\" second=\"47\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"197\" second=\"92\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"197\" second=\"39\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"197\" second=\"34\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"315\" second=\"356\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"315\" second=\"538\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"197\" second=\"35\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"197\" second=\"42\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"197\" second=\"174\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"315\" second=\"354\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"197\" second=\"354\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"197\" second=\"538\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"197\" second=\"356\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"197\" second=\"358\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"315\" second=\"174\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"315\" second=\"42\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"315\" second=\"34\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"315\" second=\"39\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"199\" second=\"80\" amount=\"2\" />																																	"	\
	<< "	<kerning first=\"199\" second=\"125\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"199\" second=\"124\" amount=\"2\" />																																	"	\
	<< "	<kerning first=\"199\" second=\"166\" amount=\"2\" />																																	"	\
	<< "	<kerning first=\"199\" second=\"92\" amount=\"2\" />																																	"	\
	<< "	<kerning first=\"199\" second=\"95\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"315\" second=\"92\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"199\" second=\"63\" amount=\"2\" />																																	"	\
	<< "	<kerning first=\"199\" second=\"39\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"199\" second=\"34\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"199\" second=\"42\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"199\" second=\"64\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"199\" second=\"174\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"315\" second=\"87\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"199\" second=\"238\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"315\" second=\"86\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"315\" second=\"84\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"208\" second=\"67\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"208\" second=\"95\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"313\" second=\"372\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"208\" second=\"46\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"208\" second=\"44\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"313\" second=\"358\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"313\" second=\"356\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"313\" second=\"538\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"321\" second=\"84\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"321\" second=\"87\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"321\" second=\"354\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"321\" second=\"538\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"321\" second=\"356\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"321\" second=\"358\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"321\" second=\"372\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"313\" second=\"354\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"313\" second=\"174\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"313\" second=\"42\" amount=\"-8\" />																																	"	\
	<< "	<kerning first=\"313\" second=\"34\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"313\" second=\"39\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"313\" second=\"92\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"313\" second=\"87\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"210\" second=\"67\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"210\" second=\"95\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"313\" second=\"86\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"210\" second=\"46\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"210\" second=\"44\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"313\" second=\"84\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"312\" second=\"511\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"337\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"211\" second=\"67\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"211\" second=\"95\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"312\" second=\"335\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"211\" second=\"46\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"211\" second=\"44\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"312\" second=\"333\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"291\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"289\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"212\" second=\"67\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"212\" second=\"95\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"312\" second=\"287\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"212\" second=\"46\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"212\" second=\"44\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"312\" second=\"285\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"283\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"281\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"213\" second=\"67\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"213\" second=\"95\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"312\" second=\"279\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"213\" second=\"46\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"213\" second=\"44\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"312\" second=\"277\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"275\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"273\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"214\" second=\"67\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"214\" second=\"95\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"312\" second=\"271\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"214\" second=\"46\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"214\" second=\"44\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"312\" second=\"269\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"265\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"263\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"216\" second=\"67\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"216\" second=\"95\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"312\" second=\"509\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"216\" second=\"46\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"216\" second=\"44\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"312\" second=\"507\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"261\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"259\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"222\" second=\"67\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"222\" second=\"95\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"312\" second=\"257\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"222\" second=\"46\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"222\" second=\"44\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"312\" second=\"339\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"248\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"246\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"352\" second=\"84\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"352\" second=\"86\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"352\" second=\"95\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"312\" second=\"245\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"352\" second=\"46\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"352\" second=\"44\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"312\" second=\"244\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"243\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"242\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"240\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"235\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"352\" second=\"42\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"352\" second=\"174\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"312\" second=\"234\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"352\" second=\"354\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"352\" second=\"356\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"352\" second=\"358\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"312\" second=\"233\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"232\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"231\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"230\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"240\" second=\"92\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"242\" second=\"92\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"242\" second=\"95\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"312\" second=\"229\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"243\" second=\"92\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"243\" second=\"95\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"312\" second=\"228\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"244\" second=\"92\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"244\" second=\"95\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"312\" second=\"227\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"245\" second=\"92\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"245\" second=\"95\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"312\" second=\"226\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"246\" second=\"92\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"246\" second=\"95\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"312\" second=\"225\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"248\" second=\"92\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"254\" second=\"92\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"223\" second=\"92\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"256\" second=\"84\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"256\" second=\"86\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"256\" second=\"47\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"256\" second=\"92\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"256\" second=\"39\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"256\" second=\"34\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"312\" second=\"224\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"45\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"256\" second=\"35\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"256\" second=\"42\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"256\" second=\"174\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"312\" second=\"92\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"256\" second=\"354\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"256\" second=\"538\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"256\" second=\"356\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"256\" second=\"358\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"312\" second=\"120\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"113\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"111\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"103\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"258\" second=\"84\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"258\" second=\"86\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"258\" second=\"47\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"258\" second=\"92\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"258\" second=\"39\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"258\" second=\"34\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"312\" second=\"101\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"312\" second=\"100\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"258\" second=\"35\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"258\" second=\"42\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"258\" second=\"174\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"312\" second=\"99\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"258\" second=\"354\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"258\" second=\"538\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"258\" second=\"356\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"258\" second=\"358\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"312\" second=\"97\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"311\" second=\"511\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"337\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"335\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"260\" second=\"84\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"260\" second=\"86\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"260\" second=\"47\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"260\" second=\"92\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"260\" second=\"39\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"260\" second=\"34\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"311\" second=\"333\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"291\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"260\" second=\"35\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"260\" second=\"42\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"260\" second=\"174\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"311\" second=\"289\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"260\" second=\"354\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"260\" second=\"538\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"260\" second=\"356\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"260\" second=\"358\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"311\" second=\"287\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"285\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"283\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"281\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"506\" second=\"84\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"506\" second=\"86\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"506\" second=\"47\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"506\" second=\"92\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"506\" second=\"39\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"506\" second=\"34\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"311\" second=\"279\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"277\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"506\" second=\"35\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"506\" second=\"42\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"506\" second=\"174\" amount=\"-4\" />																																	"
	<< "	<kerning first=\"311\" second=\"275\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"506\" second=\"354\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"506\" second=\"538\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"506\" second=\"356\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"506\" second=\"358\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"311\" second=\"273\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"271\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"269\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"265\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"262\" second=\"80\" amount=\"2\" />																																	"	\
	<< "	<kerning first=\"262\" second=\"125\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"262\" second=\"124\" amount=\"2\" />																																	"	\
	<< "	<kerning first=\"262\" second=\"166\" amount=\"2\" />																																	"	\
	<< "	<kerning first=\"262\" second=\"92\" amount=\"2\" />																																	"	\
	<< "	<kerning first=\"262\" second=\"95\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"311\" second=\"263\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"262\" second=\"63\" amount=\"2\" />																																	"	\
	<< "	<kerning first=\"262\" second=\"39\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"262\" second=\"34\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"262\" second=\"42\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"262\" second=\"64\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"262\" second=\"174\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"311\" second=\"509\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"262\" second=\"238\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"311\" second=\"507\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"261\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"264\" second=\"80\" amount=\"2\" />																																	"	\
	<< "	<kerning first=\"264\" second=\"125\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"264\" second=\"124\" amount=\"2\" />																																	"	\
	<< "	<kerning first=\"264\" second=\"166\" amount=\"2\" />																																	"	\
	<< "	<kerning first=\"264\" second=\"92\" amount=\"2\" />																																	"	\
	<< "	<kerning first=\"264\" second=\"95\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"311\" second=\"259\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"264\" second=\"63\" amount=\"2\" />																																	"	\
	<< "	<kerning first=\"264\" second=\"39\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"264\" second=\"34\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"264\" second=\"42\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"264\" second=\"64\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"264\" second=\"174\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"311\" second=\"257\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"264\" second=\"238\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"311\" second=\"339\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"248\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"266\" second=\"125\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"266\" second=\"39\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"266\" second=\"34\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"266\" second=\"42\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"266\" second=\"64\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"266\" second=\"174\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"311\" second=\"246\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"266\" second=\"238\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"268\" second=\"80\" amount=\"2\" />																																	"	\
	<< "	<kerning first=\"268\" second=\"125\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"268\" second=\"124\" amount=\"2\" />																																	"	\
	<< "	<kerning first=\"268\" second=\"166\" amount=\"2\" />																																	"	\
	<< "	<kerning first=\"268\" second=\"92\" amount=\"2\" />																																	"	\
	<< "	<kerning first=\"268\" second=\"95\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"311\" second=\"245\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"268\" second=\"63\" amount=\"2\" />																																	"	\
	<< "	<kerning first=\"268\" second=\"39\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"268\" second=\"34\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"268\" second=\"42\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"268\" second=\"64\" amount=\"-4\" />																																	"	\
	<< "	<kerning first=\"268\" second=\"174\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"311\" second=\"244\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"268\" second=\"238\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"311\" second=\"243\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"242\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"270\" second=\"67\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"270\" second=\"95\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"311\" second=\"240\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"270\" second=\"46\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"270\" second=\"44\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"311\" second=\"235\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"234\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"233\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"270\" second=\"42\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"271\" second=\"98\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"271\" second=\"104\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"271\" second=\"105\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"271\" second=\"106\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"271\" second=\"107\" amount=\"2\" />																																	"	\
	<< "	<kerning first=\"271\" second=\"108\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"271\" second=\"318\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"272\" second=\"67\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"272\" second=\"95\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"311\" second=\"232\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"272\" second=\"46\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"272\" second=\"44\" amount=\"-2\" />																																	"	\
	<< "	<kerning first=\"311\" second=\"231\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"230\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"229\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"306\" second=\"46\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"306\" second=\"44\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"311\" second=\"228\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"227\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"226\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"308\" second=\"84\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"308\" second=\"99\" amount=\"-1\" />																																	"	\
	<< "	<kerning first=\"308\" second=\"100\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"101\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"103\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"111\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"113\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"115\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"117\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"119\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"41\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"308\" second=\"93\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"308\" second=\"125\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"308\" second=\"46\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"308\" second=\"44\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"311\" second=\"225\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"308\" second=\"33\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"308\" second=\"63\" amount=\"9\" />																																	"	\
	<< "	<kerning first=\"308\" second=\"39\" amount=\"10\" />																																	"	\
	<< "	<kerning first=\"308\" second=\"34\" amount=\"10\" />																																	"	\
	<< "	<kerning first=\"311\" second=\"224\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"45\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"311\" second=\"92\" amount=\"-6\" />																																	"	\
	<< "	<kerning first=\"311\" second=\"120\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"308\" second=\"231\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"232\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"233\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"234\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"235\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"240\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"242\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"243\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"244\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"245\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"246\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"248\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"339\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"353\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"249\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"250\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"251\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"252\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"263\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"265\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"269\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"271\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"273\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"275\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"277\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"279\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"281\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"283\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"285\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"287\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"291\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"333\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"335\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"337\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"511\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"347\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"349\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"351\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"537\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"354\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"308\" second=\"538\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"308\" second=\"356\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"308\" second=\"358\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"308\" second=\"361\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"363\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"365\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"367\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"369\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"371\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"308\" second=\"373\" amount=\"-2\" />																																	"
	<< "	<kerning first=\"311\" second=\"113\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"111\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"103\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"101\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"100\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"311\" second=\"99\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"311\" second=\"97\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"310\" second=\"511\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"510\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"337\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"336\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"335\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"334\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"333\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"332\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"291\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"290\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"287\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"286\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"285\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"284\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"283\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"281\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"279\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"277\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"275\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"273\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"272\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"67\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"310\" second=\"71\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"310\" second=\"79\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"310\" second=\"81\" amount=\"-5\" />																																	"	\
	<< "	<kerning first=\"310\" second=\"99\" amount=\"-3\" />																																	"	\
	<< "	<kerning first=\"310\" second=\"100\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"101\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"103\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"106\" amount=\"3\" />																																	"	\
	<< "	<kerning first=\"310\" second=\"111\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"113\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"95\" amount=\"4\" />																																	"	\
	<< "	<kerning first=\"310\" second=\"271\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"46\" amount=\"6\" />																																	"	\
	<< "	<kerning first=\"310\" second=\"44\" amount=\"6\" />																																	"	\
	<< "	<kerning first=\"310\" second=\"58\" amount=\"6\" />																																	"	\
	<< "	<kerning first=\"310\" second=\"59\" amount=\"6\" />																																	"	\
	<< "	<kerning first=\"310\" second=\"270\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"33\" amount=\"6\" />																																	"	\
	<< "	<kerning first=\"310\" second=\"63\" amount=\"6\" />																																	"	\
	<< "	<kerning first=\"310\" second=\"269\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"268\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"265\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"264\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"171\" amount=\"5\" />																																	"	\
	<< "	<kerning first=\"310\" second=\"187\" amount=\"6\" />																																	"	\
	<< "	<kerning first=\"310\" second=\"208\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"210\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"211\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"212\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"213\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"214\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"216\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"338\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"222\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"231\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"232\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"233\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"234\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"235\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"240\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"242\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"243\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"244\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"245\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"246\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"248\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"339\" amount=\"-3\" />																																	"
	<< "	<kerning first=\"310\" second=\"262\" amount=\"-5\" />																																	"
	<< "	<kerning first=\"310\" second=\"263\" amount=\"-3\" />																																	"
	<< "	</kernings>																																												"
	<< "	</font>";
#pragma endregion Buxton_Nomal_Xml
}


void BitmapFont::ParsingXmlDataAndLoadTexture(BuildInFont bf , const std::string& zipFilePath)
{
	TiXmlElement* root = m_metaDoc->RootElement();
	if(!root)
	{
		std::string errorInfo("Failed to load root in file.");
		errorInfo = "Error , can't find root element of meta-data file : " + errorInfo + "\n\n";
		MessageBoxA( NULL , (LPCSTR)errorInfo.c_str() , "BMP Fonts Metadata Analysis Failed!", MB_ICONERROR | MB_OK );
	}

	Vec2f imageSize;
	TiXmlElement* node = NULL;
	node = root->FirstChildElement("common");
	imageSize.x = (float)atoi(node->Attribute("scaleW"));
	imageSize.y = (float)atoi(node->Attribute("scaleH"));
	m_fontHeight = atoi(node->Attribute("lineHeight"));
	node = root->FirstChildElement("pages");
	node = node->FirstChildElement("page");
	std::string filaName(node->Attribute("file"));

	if(bf == NONE)
	{
		if(zipFilePath.size() != 0)
			m_glyphSheet = new Texture("./Data/BMPFonts/" + filaName , zipFilePath);
		else
			m_glyphSheet = new Texture("./Data/BMPFonts/" + filaName );

		if (m_glyphSheet->m_textureID == -1)
		{
			std::string errorInfo(filaName);
			errorInfo = "Failed to load file : " + errorInfo;
			MessageBoxA(NULL, errorInfo.c_str(), "Failed to loading files", MB_ICONERROR | MB_OK);
		}
	}
	else
		BindBuildInTexture(bf);

	Vec2f oneOverImageSize(1.0f/imageSize.x , 1.0f/imageSize.y);
	float oneOverFontHeight = 1.0f/m_fontHeight;

	GlyphMetaData data;
	for(TiXmlElement* elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
	{
		std::string elemName = elem->Value();
		if(elemName == "chars")
		{
			for(TiXmlElement* sub_elem = elem->FirstChildElement(); sub_elem != NULL; sub_elem = sub_elem->NextSiblingElement())
			{
				int character_id = atoi(sub_elem->Attribute("id"));
				float min_x = (float)atoi(sub_elem->Attribute("x"));
				float min_y = (float)atoi(sub_elem->Attribute("y"));
				float max_x = min_x + atoi(sub_elem->Attribute("width"));
				float max_y = min_y + atoi(sub_elem->Attribute("height"));

				GlyphMetaData data;
				data.m_textCoords.min = Vec2f(min_x * oneOverImageSize.x , min_y * oneOverImageSize.y);
				data.m_textCoords.max = Vec2f(max_x * oneOverImageSize.x , max_y * oneOverImageSize.y);
				data.m_ttfA = (float)(( atoi(sub_elem->Attribute("xoffset")))* oneOverFontHeight);
				data.m_ttfB = (float)(( atoi(sub_elem->Attribute("width"))) * oneOverFontHeight);
				data.m_ttfC = (float)(( atoi(sub_elem->Attribute("xadvance")) - atoi(sub_elem->Attribute("width")) - atoi(sub_elem->Attribute("xoffset"))) * oneOverFontHeight);

				m_glyphData[character_id] = data;
				//std::string sub_elemName = sub_elem->Attribute("x");
				//MessageBoxA( NULL , (LPCSTR)sub_elemName.c_str() , "File Name is....", MB_ICONERROR | MB_OK );
			}
		}
	}
}


void BitmapFont::BindBuildInTexture(BuildInFont bf)
{
	int textureID = -1;
	std::map<BuildInFont,int>::iterator it = m_buildInFontTextureID.find(bf);
	if(it != m_buildInFontTextureID.end())
		textureID = it->second;

	if( textureID == -1 )
	{
		//const int imageSize = 1024 * 1024 * 4 + 1;
		std::ostringstream textureData;
		switch(bf)
		{
		case ARIAL:
#pragma region Arial_Texture
#pragma endregion Arial_Texture
			break;
		}
	}
	else
	{
		OpenGLRenderer::BindTexture(textureID);
	}
}


};