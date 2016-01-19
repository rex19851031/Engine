//
// DebugDrawSphere.draw() function code , circleTable code is from Ephanov at Guildhall SMU 
//
#include "Engine\Core\HenryFunctions.hpp"
#include "Engine\Renderer\OpenGLRenderer.hpp"
#include "Engine\Renderer\glext.h"
#include "Engine\Parsing\ZipUtils\ZipHelper.hpp"

#include <atlconv.h>


namespace Henry
{

#define UNUSED(x) (void)(x);

PFNGLGENBUFFERSPROC					glGenBuffers		= nullptr;
PFNGLBINDBUFFERPROC					glBindBuffer		= nullptr;
PFNGLBUFFERDATAPROC					glBufferData		= nullptr;
PFNGLGENERATEMIPMAPPROC				glGenerateMipmap	= nullptr;
PFNGLDELETEBUFFERSPROC				glDeleteBuffers		= nullptr;

PFNGLCREATESHADERPROC				glCreateShader				= nullptr;
PFNGLSHADERSOURCEPROC				glShaderSource				= nullptr;
PFNGLCOMPILESHADERPROC				glCompileShader				= nullptr;
PFNGLGETSHADERIVPROC				glGetShaderiv				= nullptr;
PFNGLGETSHADERINFOLOGPROC			glGetShaderInfoLog			= nullptr;
PFNGLGETPROGRAMINFOLOGPROC			glGetProgramInfoLog			= nullptr;
PFNGLCREATEPROGRAMPROC				glCreateProgram				= nullptr;
PFNGLATTACHSHADERPROC				glAttachShader				= nullptr;
PFNGLLINKPROGRAMPROC				glLinkProgram				= nullptr;
PFNGLGETPROGRAMIVPROC				glGetProgramiv				= nullptr;
PFNGLUSEPROGRAMPROC					glUseProgram				= nullptr;
PFNGLACTIVETEXTUREPROC				glActiveTexture				= nullptr;
PFNGLGETUNIFORMLOCATIONPROC			glGetUniformLocation		= nullptr;
PFNGLUNIFORM1FPROC					glUniform1f					= nullptr;
PFNGLUNIFORM1IPROC					glUniform1i					= nullptr;
PFNGLUNIFORM2FPROC					glUniform2f					= nullptr;
PFNGLUNIFORMMATRIX4FVARBPROC		glUniformMatrix4fv			= nullptr;
PFNGLGETATTRIBLOCATIONPROC			glGetAttribLocation			= nullptr;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC	glEnableVertexAttribArray	= nullptr;
PFNGLVERTEXATTRIBPOINTERPROC		glVertexAttribPointer		= nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC	glDisableVertexAttribArray	= nullptr;
PFNGLUNIFORM1FVPROC					glUniform1fv				= nullptr;
PFNGLUNIFORM2FVPROC					glUniform2fv				= nullptr;
PFNGLUNIFORM3FVPROC					glUniform3fv				= nullptr;
PFNGLUNIFORM4FVPROC					glUniform4fv				= nullptr;
PFNGLGENFRAMEBUFFERSPROC			glGenFramebuffers			= nullptr;
PFNGLFRAMEBUFFERTEXTURE2DPROC		glFramebufferTexture2D		= nullptr;
PFNGLBINDFRAMEBUFFERPROC			glBindFramebuffer			= nullptr;

GLenum OpenGLRenderer::VERTEX_SHADER;
GLenum OpenGLRenderer::FRAGMENT_SHADER;
GLenum OpenGLRenderer::LINK_STATUS;
GLenum OpenGLRenderer::SHAPE_QUADS;
GLenum OpenGLRenderer::SHAPE_LINES;
GLenum OpenGLRenderer::SHAPE_POINTS;
GLenum OpenGLRenderer::VERSION;
GLenum OpenGLRenderer::SHADING_LANGUAGE_VERSION;
GLenum OpenGLRenderer::ONE_MINUS_DESTINATION_COLOR;
GLenum OpenGLRenderer::ZERO;					
GLenum OpenGLRenderer::SRC_ALPHA;				
GLenum OpenGLRenderer::ONE_MINUS_SRC_ALPHA;
GLenum OpenGLRenderer::ONE;
GLenum OpenGLRenderer::SHAPE_TRIANGLES;

int OpenGLRenderer::SHADER_PROGRAM_ID;
int OpenGLRenderer::DIFFUSE_TEXTURE_ID;
int OpenGLRenderer::NORMAL_TEXTURE_ID;
int OpenGLRenderer::SPECULAR_TEXTURE_ID;

std::vector<Matrix4> OpenGLRenderer::WorldToClipMatrixStack;
std::vector<Matrix4> OpenGLRenderer::ObjectToWorldMatrixStack;

Vec2i OpenGLRenderer::WINDOW_SIZE;


void OpenGLRenderer::Initialize(Vec2i windowSize)
{
	glGenBuffers		= (PFNGLGENBUFFERSPROC)			 wglGetProcAddress( "glGenBuffers" );
	glBindBuffer		= (PFNGLBINDBUFFERPROC)			 wglGetProcAddress( "glBindBuffer" );
	glBufferData		= (PFNGLBUFFERDATAPROC)			 wglGetProcAddress( "glBufferData" );
	glGenerateMipmap	= (PFNGLGENERATEMIPMAPPROC)		 wglGetProcAddress( "glGenerateMipmap" );
	glDeleteBuffers		= (PFNGLDELETEBUFFERSPROC)		 wglGetProcAddress( "glDeleteBuffers" );
															 
	glCreateShader				= (PFNGLCREATESHADERPROC)				wglGetProcAddress( "glCreateShader" );
	glShaderSource				= (PFNGLSHADERSOURCEPROC)				wglGetProcAddress( "glShaderSource" );
	glCompileShader				= (PFNGLCOMPILESHADERPROC)				wglGetProcAddress( "glCompileShader" );
	glGetShaderiv				= (PFNGLGETSHADERIVPROC)				wglGetProcAddress( "glGetShaderiv" );
	glGetShaderInfoLog			= (PFNGLGETSHADERINFOLOGPROC)			wglGetProcAddress( "glGetShaderInfoLog" );
	glGetProgramInfoLog			= (PFNGLGETPROGRAMINFOLOGPROC)			wglGetProcAddress( "glGetProgramInfoLog" );
	glCreateProgram				= (PFNGLCREATEPROGRAMPROC)				wglGetProcAddress( "glCreateProgram" );
	glAttachShader				= (PFNGLATTACHSHADERPROC)				wglGetProcAddress( "glAttachShader" );
	glLinkProgram				= (PFNGLLINKPROGRAMPROC)				wglGetProcAddress( "glLinkProgram" );
	glGetProgramiv				= (PFNGLGETPROGRAMIVPROC)				wglGetProcAddress( "glGetProgramiv" );
	glUseProgram				= (PFNGLUSEPROGRAMPROC)					wglGetProcAddress( "glUseProgram" );
	glActiveTexture				= (PFNGLACTIVETEXTUREPROC)				wglGetProcAddress( "glActiveTexture" );
	glGetUniformLocation		= (PFNGLGETUNIFORMLOCATIONPROC)			wglGetProcAddress( "glGetUniformLocation" );
	glUniform1f					= (PFNGLUNIFORM1FPROC)					wglGetProcAddress( "glUniform1f" );
	glUniform1i					= (PFNGLUNIFORM1IPROC)					wglGetProcAddress( "glUniform1i" );
	glUniform2f					= (PFNGLUNIFORM2FPROC)					wglGetProcAddress( "glUniform2f" );				
	glUniformMatrix4fv			= (PFNGLUNIFORMMATRIX4FVARBPROC)		wglGetProcAddress( "glUniformMatrix4fv" );
	glGetAttribLocation			= (PFNGLGETATTRIBLOCATIONPROC)			wglGetProcAddress( "glGetAttribLocation" );	 
	glEnableVertexAttribArray	= (PFNGLENABLEVERTEXATTRIBARRAYARBPROC) wglGetProcAddress( "glEnableVertexAttribArray" );
	glVertexAttribPointer		= (PFNGLVERTEXATTRIBPOINTERPROC)		wglGetProcAddress( "glVertexAttribPointer" );			
	glDisableVertexAttribArray	= (PFNGLDISABLEVERTEXATTRIBARRAYPROC)	wglGetProcAddress( "glDisableVertexAttribArray" );
	glUniform1fv				= (PFNGLUNIFORM1FVPROC)					wglGetProcAddress( "glUniform1fv" );
	glUniform2fv				= (PFNGLUNIFORM2FVPROC)					wglGetProcAddress( "glUniform2fv" );
	glUniform3fv				= (PFNGLUNIFORM3FVPROC)					wglGetProcAddress( "glUniform3fv" );
	glUniform4fv				= (PFNGLUNIFORM4FVPROC)					wglGetProcAddress( "glUniform4fv" );
	
	glGenFramebuffers			= (PFNGLGENFRAMEBUFFERSPROC)			wglGetProcAddress( "glGenFramebuffers" );
	glFramebufferTexture2D		= (PFNGLFRAMEBUFFERTEXTURE2DPROC)		wglGetProcAddress( "glFramebufferTexture2D" );
	glBindFramebuffer			= (PFNGLBINDFRAMEBUFFERPROC)			wglGetProcAddress( "glBindFramebuffer" );

	VERTEX_SHADER				= GL_VERTEX_SHADER;
	FRAGMENT_SHADER				= GL_FRAGMENT_SHADER;
	LINK_STATUS					= GL_LINK_STATUS;
	SHAPE_QUADS					= GL_QUADS;
	SHAPE_LINES					= GL_LINES;
	SHAPE_POINTS				= GL_POINTS;
	SHAPE_TRIANGLES				= GL_TRIANGLES;
	VERSION						= GL_VERSION;
	SHADING_LANGUAGE_VERSION	= GL_SHADING_LANGUAGE_VERSION;
	DIFFUSE_TEXTURE_ID			= CreateDefaultDiffuseTexture();
	NORMAL_TEXTURE_ID			= CreateDefaultNormalTexture();
	SPECULAR_TEXTURE_ID			= CreateDefaultSpecularTexture();

	ONE_MINUS_DESTINATION_COLOR		= GL_ONE_MINUS_DST_COLOR;
	ZERO							= GL_ZERO;
	SRC_ALPHA						= GL_SRC_ALPHA;
	ONE_MINUS_SRC_ALPHA				= GL_ONE_MINUS_SRC_ALPHA;
	ONE								= GL_ONE;
	
	WINDOW_SIZE	= windowSize;

	WorldToClipMatrixStack.push_back( Matrix4() );
	ObjectToWorldMatrixStack.push_back( Matrix4() );

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

//	GenerateMipMap(5);

// 	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
// 	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
// 
// 	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
// 	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

//	WHITE_TEXTURE_ID = CreateWhiteTexture();

// 	glEnable(GL_LINE_SMOOTH);
// 	glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
// 
// 	glEnable(GL_BLEND);
// 	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void OpenGLRenderer::BindTexture(int textureID, int textureUnit)
{
	glActiveTexture( GL_TEXTURE0 + textureUnit );
	glEnable( GL_TEXTURE_2D );
	glBindTexture(GL_TEXTURE_2D,textureID);
}


void OpenGLRenderer::BindTexture(Texture* diffuse , Texture* emissive , Texture* normal , Texture* specular)
{
	if(emissive)
	{
		glActiveTexture(GL_TEXTURE3);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, emissive->m_textureID);
	}

	if(specular)
	{
		glActiveTexture(GL_TEXTURE2);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, specular->m_textureID);
	}

	if(normal)
	{
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, normal->m_textureID);
	}

	if(diffuse)
	{
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, diffuse->m_textureID);
	}
}


void OpenGLRenderer::BindWhiteTexture()
{
	BindTexture(DIFFUSE_TEXTURE_ID);
}


void OpenGLRenderer::BindFrameBuffer(int bufferID)
{
	glBindFramebuffer( GL_FRAMEBUFFER, bufferID );
}


int OpenGLRenderer::GetFrameBufferID()
{
	int currentFBOID;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBOID);
	return currentFBOID;
}


void OpenGLRenderer::FrameBufferColorTexture(int textureID, int textureUnit)
{
	glFramebufferTexture2D( GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT0 + textureUnit , GL_TEXTURE_2D , textureID , 0 );
	glActiveTexture( GL_TEXTURE0 + textureUnit );
}


void OpenGLRenderer::BlendMode( GLenum blendSource, GLenum blendDestination )
{
	glBlendFunc( blendSource, blendDestination );
}


int OpenGLRenderer::CreateBlankTexture(const Vec2i& size)
{
	int openglTextureID = 0;
	unsigned char* imageData = new unsigned char[ size.x * size.y ];
	memset( imageData, 255, size.x * size.y );

	// Enable texturing
	glEnable( GL_TEXTURE_2D );

	// Tell OpenGL that our pixel data is single-byte aligned
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	// Ask OpenGL for an unused texName (ID number) to use for this texture
	glGenTextures( 1, (GLuint*) &openglTextureID );

	// Tell OpenGL to bind (set) this as the currently active texture
	glBindTexture( GL_TEXTURE_2D, openglTextureID );

	// Set texture clamp vs. wrap (repeat)
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT ); // one of: GL_CLAMP_TO_EDGE, GL_REPEAT, GL_MIRRORED_REPEAT, GL_MIRROR_CLAMP_TO_EDGE, ...
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ); // one of: GL_CLAMP_TO_EDGE, GL_REPEAT, GL_MIRRORED_REPEAT, GL_MIRROR_CLAMP_TO_EDGE, ...

	// Set magnification (texel > pixel) and minification (texel < pixel) filters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); // one of: GL_NEAREST, GL_LINEAR
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ); // one of: GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR

	GLenum bufferFormat = GL_RGBA; // the format our source pixel data is currently in; any of: GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, ...
	GLenum internalFormat = bufferFormat; // the format we want the texture to me on the card; allows us to translate into a different texture format as we upload to OpenGL

	glTexImage2D(			// Upload this pixel data to our new OpenGL texture
		GL_TEXTURE_2D,		// Creating this as a 2d texture
		0,					// Which mipmap level to use as the "root" (0 = the highest-quality, full-res image), if mipmaps are enabled
		internalFormat,		// Type of texel format we want OpenGL to use for this texture internally on the video card
		1,					// Texel-width of image; for maximum compatibility, use 2^N + 2^B, where N is some integer in the range [3,10], and B is the border thickness [0,1]
		1,					// Texel-height of image; for maximum compatibility, use 2^M + 2^B, where M is some integer in the range [3,10], and B is the border thickness [0,1]
		0,					// Border size, in texels (must be 0 or 1)
		bufferFormat,		// Pixel format describing the composition of the pixel data in buffer
		GL_UNSIGNED_BYTE,	// Pixel color components are unsigned bytes (one byte per color/alpha channel)
		imageData );		// Location of the actual pixel data bytes/buffer

	delete imageData;
	return openglTextureID;
}


int OpenGLRenderer::LoadTexture( const std::string& imageFilePath , Vec2i& size )
{
	int openglTextureID = 0;

	int numComponents = 0; // Filled in for us to indicate how many color/alpha components the image had (e.g. 3=RGB, 4=RGBA)
	int numComponentsRequested = 0; // don't care; we support 3 (RGB) or 4 (RGBA)
	unsigned char* imageData = stbi_load( imageFilePath.c_str(), &size.x, &size.y, &numComponents, numComponentsRequested );
	
	if(!imageData)
		return -1;

	// Enable texturing
	glEnable( GL_TEXTURE_2D );

	// Tell OpenGL that our pixel data is single-byte aligned
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	// Ask OpenGL for an unused texName (ID number) to use for this texture
	glGenTextures( 1, (GLuint*) &openglTextureID );

	// Tell OpenGL to bind (set) this as the currently active texture
	glBindTexture( GL_TEXTURE_2D, openglTextureID );

	// Set texture clamp vs. wrap (repeat)
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT ); // one of: GL_CLAMP_TO_EDGE, GL_REPEAT, GL_MIRRORED_REPEAT, GL_MIRROR_CLAMP_TO_EDGE, ...
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ); // one of: GL_CLAMP_TO_EDGE, GL_REPEAT, GL_MIRRORED_REPEAT, GL_MIRROR_CLAMP_TO_EDGE, ...

	// Set magnification (texel > pixel) and minification (texel < pixel) filters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); // one of: GL_NEAREST, GL_LINEAR
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ); // one of: GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR

	GLenum bufferFormat = GL_RGBA; // the format our source pixel data is currently in; any of: GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, ...
	if( numComponents == 3 )
		bufferFormat = GL_RGB;

	// Todo: What happens if numComponents is neither 3 nor 4?

	GLenum internalFormat = bufferFormat; // the format we want the texture to me on the card; allows us to translate into a different texture format as we upload to OpenGL

	glTexImage2D(			// Upload this pixel data to our new OpenGL texture
		GL_TEXTURE_2D,		// Creating this as a 2d texture
		0,					// Which mipmap level to use as the "root" (0 = the highest-quality, full-res image), if mipmaps are enabled
		internalFormat,		// Type of texel format we want OpenGL to use for this texture internally on the video card
		size.x,			// Texel-width of image; for maximum compatibility, use 2^N + 2^B, where N is some integer in the range [3,10], and B is the border thickness [0,1]
		size.y,			// Texel-height of image; for maximum compatibility, use 2^M + 2^B, where M is some integer in the range [3,10], and B is the border thickness [0,1]
		0,					// Border size, in texels (must be 0 or 1)
		bufferFormat,		// Pixel format describing the composition of the pixel data in buffer
		GL_UNSIGNED_BYTE,	// Pixel color components are unsigned bytes (one byte per color/alpha channel)
		imageData );		// Location of the actual pixel data bytes/buffer

	stbi_image_free( imageData );
	// glGenMipmaps here (if this texture needs them)
	return openglTextureID;
}


int OpenGLRenderer::LoadTexture(const std::string& imageFilePath , Vec2i& size , const std::string& zipFilePath)
{
	int len;
	bool success;
	const unsigned char* data = ZipHelper::GetContentInZip(zipFilePath,imageFilePath,"c23",&len,&success);
	int openglTextureID = 0;
	int numComponents = 0; // Filled in for us to indicate how many color/alpha components the image had (e.g. 3=RGB, 4=RGBA)
	int numComponentsRequested = 0; // don't care; we support 3 (RGB) or 4 (RGBA)

	unsigned char* imageData = stbi_load_from_memory( data , len , &size.x , &size.y , &numComponents , numComponentsRequested);

	if(!success)
		return -1;

	// Enable texturing
	glEnable( GL_TEXTURE_2D );

	// Tell OpenGL that our pixel data is single-byte aligned
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	// Ask OpenGL for an unused texName (ID number) to use for this texture
	glGenTextures( 1, (GLuint*) &openglTextureID );

	// Tell OpenGL to bind (set) this as the currently active texture
	glBindTexture( GL_TEXTURE_2D, openglTextureID );

	// Set texture clamp vs. wrap (repeat)
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT ); // one of: GL_CLAMP_TO_EDGE, GL_REPEAT, GL_MIRRORED_REPEAT, GL_MIRROR_CLAMP_TO_EDGE, ...
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ); // one of: GL_CLAMP_TO_EDGE, GL_REPEAT, GL_MIRRORED_REPEAT, GL_MIRROR_CLAMP_TO_EDGE, ...

	// Set magnification (texel > pixel) and minification (texel < pixel) filters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); // one of: GL_NEAREST, GL_LINEAR
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ); // one of: GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR

	GLenum bufferFormat = GL_RGBA; // the format our source pixel data is currently in; any of: GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, ...
	if( numComponents == 3 )
		bufferFormat = GL_RGB;

	// Todo: What happens if numComponents is neither 3 nor 4?

	GLenum internalFormat = bufferFormat; // the format we want the texture to me on the card; allows us to translate into a different texture format as we upload to OpenGL

	glTexImage2D(			// Upload this pixel data to our new OpenGL texture
		GL_TEXTURE_2D,		// Creating this as a 2d texture
		0,					// Which mipmap level to use as the "root" (0 = the highest-quality, full-res image), if mipmaps are enabled
		internalFormat,		// Type of texel format we want OpenGL to use for this texture internally on the video card
		size.x,			// Texel-width of image; for maximum compatibility, use 2^N + 2^B, where N is some integer in the range [3,10], and B is the border thickness [0,1]
		size.y,			// Texel-height of image; for maximum compatibility, use 2^M + 2^B, where M is some integer in the range [3,10], and B is the border thickness [0,1]
		0,					// Border size, in texels (must be 0 or 1)
		bufferFormat,		// Pixel format describing the composition of the pixel data in buffer
		GL_UNSIGNED_BYTE,	// Pixel color components are unsigned bytes (one byte per color/alpha channel)
		imageData );		// Location of the actual pixel data bytes/buffer

	stbi_image_free( imageData );
	// glGenMipmaps here (if this texture needs them)
	return openglTextureID;
 }


int OpenGLRenderer::LoadTextureFromData( const unsigned char* imageData , Vec2i& size )
{
	int openglTextureID = 0;

	int numComponents = 4; // Filled in for us to indicate how many color/alpha components the image had (e.g. 3=RGB, 4=RGBA)
	//int numComponentsRequested = 0; // don't care; we support 3 (RGB) or 4 (RGBA)

	if(!imageData)
		return -1;

	// Enable texturing
	glEnable( GL_TEXTURE_2D );

	// Tell OpenGL that our pixel data is single-byte aligned
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	// Ask OpenGL for an unused texName (ID number) to use for this texture
	glGenTextures( 1, (GLuint*) &openglTextureID );

	// Tell OpenGL to bind (set) this as the currently active texture
	glBindTexture( GL_TEXTURE_2D, openglTextureID );

	// Set texture clamp vs. wrap (repeat)
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT ); // one of: GL_CLAMP_TO_EDGE, GL_REPEAT, GL_MIRRORED_REPEAT, GL_MIRROR_CLAMP_TO_EDGE, ...
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ); // one of: GL_CLAMP_TO_EDGE, GL_REPEAT, GL_MIRRORED_REPEAT, GL_MIRROR_CLAMP_TO_EDGE, ...

	// Set magnification (texel > pixel) and minification (texel < pixel) filters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); // one of: GL_NEAREST, GL_LINEAR
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ); // one of: GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR

	GLenum bufferFormat = GL_RGBA; // the format our source pixel data is currently in; any of: GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, ...
	if( numComponents == 3 )
		bufferFormat = GL_RGB;

	// Todo: What happens if numComponents is neither 3 nor 4?

	GLenum internalFormat = bufferFormat; // the format we want the texture to me on the card; allows us to translate into a different texture format as we upload to OpenGL

	glTexImage2D(			// Upload this pixel data to our new OpenGL texture
		GL_TEXTURE_2D,		// Creating this as a 2d texture
		0,					// Which mipmap level to use as the "root" (0 = the highest-quality, full-res image), if mipmaps are enabled
		internalFormat,		// Type of texel format we want OpenGL to use for this texture internally on the video card
		size.x,				// Texel-width of image; for maximum compatibility, use 2^N + 2^B, where N is some integer in the range [3,10], and B is the border thickness [0,1]
		size.y,				// Texel-height of image; for maximum compatibility, use 2^M + 2^B, where M is some integer in the range [3,10], and B is the border thickness [0,1]
		0,					// Border size, in texels (must be 0 or 1)
		bufferFormat,		// Pixel format describing the composition of the pixel data in buffer
		GL_UNSIGNED_BYTE,	// Pixel color components are unsigned bytes (one byte per color/alpha channel)
		imageData );		// Location of the actual pixel data bytes/buffer

	// glGenMipmaps here (if this texture needs them)
	return openglTextureID;
}


void OpenGLRenderer::ChangeTexture(int textureID)
{
	glBindTexture(GL_TEXTURE_2D, textureID);
}


void OpenGLRenderer::ClearScreen( float r , float g , float b )
{
	glClearColor( r, g, b, 1.f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearDepth(1.0f);
};


void OpenGLRenderer::DrawVertexWithVertexArray(Vertex_PosColor* vertices,int sizeOfVertices,GLenum glDrawMode)
{
	int vertexArrayID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_position");
	int colorArrayID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_color");
	
	glDisable(GL_TEXTURE_2D);

	glEnableVertexAttribArray(vertexArrayID);
	glEnableVertexAttribArray(colorArrayID);

	glVertexAttribPointer(vertexArrayID,3,GL_FLOAT,false,sizeof(Vertex_PosColor), &vertices[0].position);
	glVertexAttribPointer(colorArrayID,4,GL_UNSIGNED_BYTE,true,sizeof(Vertex_PosColor), &vertices[0].color);

	glDrawArrays( glDrawMode, 0, sizeOfVertices );
	
	glDisableVertexAttribArray(vertexArrayID);
	glDisableVertexAttribArray(colorArrayID);

	glEnable(GL_TEXTURE_2D);
}


void OpenGLRenderer::DrawVertexWithVertexBufferObject(int vboID , int numOfVertices,GLenum glDrawMode, Vertex_PosColor)
{
	glDisable(GL_TEXTURE_2D);

	int vertexArrayID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_position");
	int colorArrayID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_color");

	glBindBuffer( GL_ARRAY_BUFFER, vboID );

	glEnableVertexAttribArray(vertexArrayID);
	glEnableVertexAttribArray(colorArrayID);

	glVertexAttribPointer(vertexArrayID,3,GL_FLOAT,false,sizeof(Vertex_PosColor), (const GLvoid*) offsetof( Vertex_PosColor, position) );
	glVertexAttribPointer(colorArrayID,4,GL_UNSIGNED_BYTE,true,sizeof(Vertex_PosColor), (const GLvoid*) offsetof( Vertex_PosColor, color) );

	glDrawArrays( glDrawMode , 0, numOfVertices );

	glDisableVertexAttribArray(vertexArrayID);
	glDisableVertexAttribArray(colorArrayID);

	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	glEnable(GL_TEXTURE_2D);
}


void OpenGLRenderer::DrawVertexWithIndexedVertexBufferObject(int vboID , int iboID , int numOfVertices , int numOfIndices , GLenum glDrawMode , Vertex_PCTN)
{
	UNUSED(numOfIndices);

	int vertexArrayID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_position");
	int colorArrayID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_color");
	int texArrayID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_textureCoords");
	int tangentID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_tangent");
	int biTangentID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_biTangent");
	int normalID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_normal");

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER , iboID);
	glBindBuffer( GL_ARRAY_BUFFER, vboID );

	glEnableVertexAttribArray(vertexArrayID);
	glEnableVertexAttribArray(colorArrayID);
	glEnableVertexAttribArray(texArrayID);
	glEnableVertexAttribArray(tangentID);
	glEnableVertexAttribArray(biTangentID);
	glEnableVertexAttribArray(normalID);

	glVertexAttribPointer(vertexArrayID,3,GL_FLOAT,false,sizeof(Vertex_PCTN), (const GLvoid*) offsetof( Vertex_PCTN, position) );
	glVertexAttribPointer(colorArrayID,4,GL_UNSIGNED_BYTE,true,sizeof(Vertex_PCTN), (const GLvoid*) offsetof( Vertex_PCTN, color) );
	glVertexAttribPointer(texArrayID,2,GL_FLOAT,false,sizeof(Vertex_PCTN), (const GLvoid*) offsetof( Vertex_PCTN, texCoords) );
	glVertexAttribPointer(tangentID,3,GL_FLOAT,false,sizeof(Vertex_PCTN), (const GLvoid*) offsetof( Vertex_PCTN, normals.tangent) );
	glVertexAttribPointer(biTangentID,3,GL_FLOAT,false,sizeof(Vertex_PCTN), (const GLvoid*) offsetof( Vertex_PCTN, normals.biTangent) );
	glVertexAttribPointer(normalID,3,GL_FLOAT,false,sizeof(Vertex_PCTN), (const GLvoid*) offsetof( Vertex_PCTN, normals.normal) );

	glDrawArrays( glDrawMode , 0, numOfVertices );

	glDisableVertexAttribArray(vertexArrayID);
	glDisableVertexAttribArray(colorArrayID);
	glDisableVertexAttribArray(texArrayID);
	glDisableVertexAttribArray(tangentID);
	glDisableVertexAttribArray(biTangentID);
	glDisableVertexAttribArray(normalID);

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0);
}


void OpenGLRenderer::DrawVertexWithVertexBufferObject(int vboID , int numOfVertices,GLenum glDrawMode, Vertex_PCTN)
{
	int vertexArrayID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_position");
	int colorArrayID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_color");
	int texArrayID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_textureCoords");
	int tangentID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_tangent");
	int biTangentID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_biTangent");
	int normalID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_normal");

	glBindBuffer( GL_ARRAY_BUFFER, vboID );

	glEnableVertexAttribArray(vertexArrayID);
	glEnableVertexAttribArray(colorArrayID);
	glEnableVertexAttribArray(texArrayID);
	glEnableVertexAttribArray(tangentID);
	glEnableVertexAttribArray(biTangentID);
	glEnableVertexAttribArray(normalID);

	glVertexAttribPointer(vertexArrayID,3,GL_FLOAT,false,sizeof(Vertex_PCTN), (const GLvoid*) offsetof( Vertex_PCTN, position) );
	glVertexAttribPointer(colorArrayID,4,GL_UNSIGNED_BYTE,true,sizeof(Vertex_PCTN), (const GLvoid*) offsetof( Vertex_PCTN, color) );
	glVertexAttribPointer(texArrayID,2,GL_FLOAT,false,sizeof(Vertex_PCTN), (const GLvoid*) offsetof( Vertex_PCTN, texCoords) );
	glVertexAttribPointer(tangentID,3,GL_FLOAT,false,sizeof(Vertex_PCTN), (const GLvoid*) offsetof( Vertex_PCTN, normals.tangent) );
	glVertexAttribPointer(biTangentID,3,GL_FLOAT,false,sizeof(Vertex_PCTN), (const GLvoid*) offsetof( Vertex_PCTN, normals.biTangent) );
	glVertexAttribPointer(normalID,3,GL_FLOAT,false,sizeof(Vertex_PCTN), (const GLvoid*) offsetof( Vertex_PCTN, normals.normal) );

	glDrawArrays( glDrawMode , 0, numOfVertices );

	glDisableVertexAttribArray(vertexArrayID);
	glDisableVertexAttribArray(colorArrayID);
	glDisableVertexAttribArray(texArrayID);
	glDisableVertexAttribArray(tangentID);
	glDisableVertexAttribArray(biTangentID);
	glDisableVertexAttribArray(normalID);

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
}


void OpenGLRenderer::DrawVertexWithVertexArray(Vertex_PCT* vertices,int sizeOfVertices,GLenum glDrawMode)
{
	glEnable(GL_TEXTURE_2D);

	int vertexArrayID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_position");
	int colorArrayID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_color");
	int texArrayID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_textureCoords");

	glEnableVertexAttribArray(vertexArrayID);
	glEnableVertexAttribArray(colorArrayID);
	glEnableVertexAttribArray(texArrayID);

	glVertexAttribPointer(vertexArrayID,3,GL_FLOAT,false,sizeof(Vertex_PCT), &vertices[0].position);
	glVertexAttribPointer(colorArrayID,4,GL_UNSIGNED_BYTE,true,sizeof(Vertex_PCT), &vertices[0].color);
	glVertexAttribPointer(texArrayID,2,GL_FLOAT,false,sizeof(Vertex_PCT), &vertices[0].texCoords);

	glDrawArrays( glDrawMode , 0, sizeOfVertices );
	
	glDisableVertexAttribArray(vertexArrayID);
	glDisableVertexAttribArray(colorArrayID);
	glDisableVertexAttribArray(texArrayID);

	glDisable(GL_TEXTURE_2D);
}


void OpenGLRenderer::DrawVertexWithVertexArray(std::vector<Vertex_PosColor>& vertices,GLenum glDrawMode)
{
	glDisable(GL_TEXTURE_2D);

	int vertexArrayID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_position");
	int colorArrayID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_color");

	glEnableVertexAttribArray(vertexArrayID);
	glEnableVertexAttribArray(colorArrayID);

	glVertexAttribPointer(vertexArrayID,3,GL_FLOAT,false,sizeof(Vertex_PosColor), &vertices[0].position);
	glVertexAttribPointer(colorArrayID,4,GL_UNSIGNED_BYTE,true,sizeof(Vertex_PosColor), &vertices[0].color);

	glDrawArrays( glDrawMode , 0, vertices.size() );

	glDisableVertexAttribArray(vertexArrayID);
	glDisableVertexAttribArray(colorArrayID);

	glEnable(GL_TEXTURE_2D);
}


void OpenGLRenderer::DrawVertexWithVertexArray(std::vector<Vertex_PCT>& vertices,GLenum glDrawMode)
{
	if(vertices.size() == 0)
		return;

	glEnable(GL_TEXTURE_2D);

	int vertexArrayID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_position");
	int colorArrayID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_color");
	int texArrayID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_textureCoords");

	glEnableVertexAttribArray(vertexArrayID);
	glEnableVertexAttribArray(colorArrayID);
	glEnableVertexAttribArray(texArrayID);

	glVertexAttribPointer(vertexArrayID,3,GL_FLOAT,false,sizeof(Vertex_PCT), &vertices[0].position);
	glVertexAttribPointer(colorArrayID,4,GL_UNSIGNED_BYTE,true,sizeof(Vertex_PCT), &vertices[0].color);
	glVertexAttribPointer(texArrayID,2,GL_FLOAT,false,sizeof(Vertex_PCT), &vertices[0].texCoords);

	glDrawArrays( glDrawMode , 0, vertices.size() );

	glDisableVertexAttribArray(vertexArrayID);
	glDisableVertexAttribArray(colorArrayID);
	glDisableVertexAttribArray(texArrayID);

	glDisable(GL_TEXTURE_2D);
}


void OpenGLRenderer::DrawVertexWithVertexArray(Vertex_PCTN* vertices,int sizeOfVertices,GLenum glDrawMode)
{
	glEnable(GL_TEXTURE_2D);

	int vertexArrayID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_position");
	int colorArrayID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_color");
	int texArrayID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_textureCoords");
	int tangentID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_tangent");
	int biTangentID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_biTangent");
	int normalID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_normal");

	glEnableVertexAttribArray(vertexArrayID);
	glEnableVertexAttribArray(colorArrayID);
	glEnableVertexAttribArray(texArrayID);
	glEnableVertexAttribArray(tangentID);
	glEnableVertexAttribArray(biTangentID);
	glEnableVertexAttribArray(normalID);

	glVertexAttribPointer(vertexArrayID,3,GL_FLOAT,false,sizeof(Vertex_PCTN), &vertices[0].position);
	glVertexAttribPointer(colorArrayID,4,GL_UNSIGNED_BYTE,true,sizeof(Vertex_PCTN), &vertices[0].color);
	glVertexAttribPointer(texArrayID,2,GL_FLOAT,false,sizeof(Vertex_PCTN), &vertices[0].texCoords);
	glVertexAttribPointer(tangentID,3,GL_FLOAT,false,sizeof(Vertex_PCTN), &vertices[0].normals.tangent);
	glVertexAttribPointer(biTangentID,3,GL_FLOAT,false,sizeof(Vertex_PCTN), &vertices[0].normals.biTangent);
	glVertexAttribPointer(normalID,3,GL_FLOAT,false,sizeof(Vertex_PCTN), &vertices[0].normals.normal);

	glDrawArrays( glDrawMode , 0, sizeOfVertices );

	glDisableVertexAttribArray(vertexArrayID);
	glDisableVertexAttribArray(colorArrayID);
	glDisableVertexAttribArray(texArrayID);
	glDisableVertexAttribArray(tangentID);
	glDisableVertexAttribArray(biTangentID);
	glDisableVertexAttribArray(normalID);

	glDisable(GL_TEXTURE_2D);
}


void OpenGLRenderer::DrawVertexWithVertexArray(std::vector<Vertex_PCTN>& vertices,GLenum glDrawMode)
{
	glEnable(GL_TEXTURE_2D);

	int vertexArrayID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_position");
	int colorArrayID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_color");
	int texArrayID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_textureCoords");
	int tangentID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_tangent");
	int biTangentID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_biTangent");
	int normalID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_normal");

	glEnableVertexAttribArray(vertexArrayID);
	glEnableVertexAttribArray(colorArrayID);
	glEnableVertexAttribArray(texArrayID);
	glEnableVertexAttribArray(tangentID);
	glEnableVertexAttribArray(biTangentID);
	glEnableVertexAttribArray(normalID);

	glVertexAttribPointer(vertexArrayID,3,GL_FLOAT,false,sizeof(Vertex_PCTN), &vertices[0].position);
	glVertexAttribPointer(colorArrayID,4,GL_UNSIGNED_BYTE,true,sizeof(Vertex_PCTN), &vertices[0].color);
	glVertexAttribPointer(texArrayID,2,GL_FLOAT,false,sizeof(Vertex_PCTN), &vertices[0].texCoords);
	glVertexAttribPointer(tangentID,3,GL_FLOAT,false,sizeof(Vertex_PCTN), &vertices[0].normals.tangent);
	glVertexAttribPointer(biTangentID,3,GL_FLOAT,false,sizeof(Vertex_PCTN), &vertices[0].normals.biTangent);
	glVertexAttribPointer(normalID,3,GL_FLOAT,false,sizeof(Vertex_PCTN), &vertices[0].normals.normal);

	glDrawArrays( glDrawMode , 0, vertices.size() );

	glDisableVertexAttribArray(vertexArrayID);
	glDisableVertexAttribArray(colorArrayID);
	glDisableVertexAttribArray(texArrayID);
	glDisableVertexAttribArray(tangentID);
	glDisableVertexAttribArray(biTangentID);
	glDisableVertexAttribArray(normalID);

	glDisable(GL_TEXTURE_2D);
}


void OpenGLRenderer::DrawSolidSphere(Vec3f center,float radius, RGBA color ,int slices,int stacks)
{
	PushMatrix();
	Translatef(center.x,center.y,center.z);
	glColor4f(color.r,color.g,color.b,color.a);
	
	int i,j;

	/* Adjust z and radius as stacks are drawn. */

	double z0,z1;
	double r0,r1;

	/* Pre-computed circle */

	double *sint1,*cost1;
	double *sint2,*cost2;
	circleTable(&sint1,&cost1,-slices);
	circleTable(&sint2,&cost2,stacks*2);

	/* The top stack is covered with a triangle fan */

	z0 = 1.0;
	z1 = cost2[1];
	r0 = 0.0;
	r1 = sint2[1];

	glBegin(GL_TRIANGLE_FAN);

	glNormal3d(0,0,1);
	glVertex3d(0,0,radius);

	for (j=slices; j>=0; j--)
	{       
		glNormal3d(cost1[j]*r1,        sint1[j]*r1,        z1       );
		glVertex3d(cost1[j]*r1*radius, sint1[j]*r1*radius, z1*radius);
	}

	glEnd();

	/* Cover each stack with a quad strip, except the top and bottom stacks */

	for( i=1; i<stacks-1; i++ )
	{
		z0 = z1; z1 = cost2[i+1];
		r0 = r1; r1 = sint2[i+1];

		glBegin(GL_QUAD_STRIP);

		for(j=0; j<=slices; j++)
		{
			glNormal3d(cost1[j]*r1,        sint1[j]*r1,        z1       );
			glVertex3d(cost1[j]*r1*radius, sint1[j]*r1*radius, z1*radius);
			glNormal3d(cost1[j]*r0,        sint1[j]*r0,        z0       );
			glVertex3d(cost1[j]*r0*radius, sint1[j]*r0*radius, z0*radius);
		}

		glEnd();
	}

	/* The bottom stack is covered with a triangle fan */

	z0 = z1;
	r0 = r1;

	glBegin(GL_TRIANGLE_FAN);

	glNormal3d(0,0,-1);
	glVertex3d(0,0,-radius);

	for (j=0; j<=slices; j++)
	{
		glNormal3d(cost1[j]*r0,          sint1[j]*r0,          z0       );
		glVertex3d(cost1[j]*r0*radius, sint1[j]*r0*radius, z0*radius);
	}

	glEnd();

	/* Release sin and cos tables */

	free(sint1);
	free(cost1);
	free(sint2);
	free(cost2);
	PopMatrix();
}


void OpenGLRenderer::DrawWireSphere(Vec3f center,float radius, RGBA color ,int slices,int stacks)
{
	PushMatrix();
	Translatef(center.x,center.y,center.z);
	glColor4f(color.r,color.g,color.b,color.a);

	int i,j;

	/* Adjust z and radius as stacks and slices are drawn. */

	double r;
	double x,y,z;

	/* Pre-computed circle */

	double *sint1,*cost1;
	double *sint2,*cost2;
	circleTable(&sint1,&cost1,-slices  );
	circleTable(&sint2,&cost2, stacks*2);

	/* Draw a line loop for each stack */

	for (i=1; i<stacks; i++)
	{
		z = cost2[i];
		r = sint2[i];

		glBegin(GL_LINE_LOOP);

		for(j=0; j<=slices; j++)
		{
			x = cost1[j];
			y = sint1[j];

			glNormal3d(x,y,z);
			glVertex3d(x*r*radius,y*r*radius,z*radius);
		}

		glEnd();
	}

	/* Draw a line loop for each slice */

	for (i=0; i<slices; i++)
	{
		glBegin(GL_LINE_STRIP);

		for(j=0; j<=stacks; j++)
		{
			x = cost1[i]*sint2[j];
			y = sint1[i]*sint2[j];
			z = cost2[j];

			glNormal3d(x,y,z);
			glVertex3d(x*radius,y*radius,z*radius);
		}

		glEnd();
	}

	/* Release sin and cos tables */

	free(sint1);
	free(cost1);
	free(sint2);
	free(cost2);

	PopMatrix();
}


void OpenGLRenderer::PointSmooth(bool value)
{
	if(value)
		glEnable(GL_POINT_SMOOTH);
	else
		glDisable(GL_POINT_SMOOTH);
}


void OpenGLRenderer::DepthTest(bool value)
{
	if(value)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
}


void OpenGLRenderer::LineWidth(float width)
{
	glLineWidth(width);
}


GLuint OpenGLRenderer::CreateProgram()
{
	return glCreateProgram();
}


void OpenGLRenderer::AttachShader(GLuint programID,GLuint shaderID)
{
	glAttachShader(programID,shaderID);
}


void OpenGLRenderer::LinkProgram(GLuint programID)
{
	glLinkProgram(programID);
}


void OpenGLRenderer::GetProgramIV(GLuint programID,GLenum name,bool &wasSuccessful)
{
	GLint success;
	
	glGetProgramiv(programID,name,&success);
	
	if(success)
		wasSuccessful = true;
	else
	{
		wasSuccessful = false;

		std::string errorMessage;

		GLint infoLogLength;
		glGetProgramiv( programID, GL_INFO_LOG_LENGTH, &infoLogLength );
		char* infoLogText = new char[ infoLogLength + 1 ];
		glGetProgramInfoLog( programID, infoLogLength, NULL, infoLogText );
		std::string infoLogString( infoLogText );
		infoLogString += "\nCurrent OpenGL version : " + OpenGLRenderer::GetString(OpenGLRenderer::VERSION) + "\n\n";
		infoLogString += "Current GLSL version : " + OpenGLRenderer::GetString(OpenGLRenderer::SHADING_LANGUAGE_VERSION) + "\n\n";
		delete[] infoLogText;
		infoLogString += "The program will close now.";
		std::string errorTitle = "SD3 Demo :: GLSL link error";// + errorTitle;
		MessageBoxA( NULL , (LPCSTR)infoLogString.c_str() , (LPCSTR)errorTitle.c_str(), MB_ICONERROR | MB_OK );
		exit(0);
	}
}


std::string OpenGLRenderer::GetString(GLenum variable)
{
	std::string version((const char*) glGetString(variable));
	return version;
}


void OpenGLRenderer::UseProgram(int programID)
{
	glUseProgram(programID);
	SHADER_PROGRAM_ID = programID;
}


GLint OpenGLRenderer::GetUniformLocation(int programID,const char* variableName)
{
	return glGetUniformLocation(programID,variableName);
}


void OpenGLRenderer::SetUniform( int uniformLocation , Vec2f value)
{
	glUniform2f(uniformLocation,value.x,value.y);
}


void OpenGLRenderer::SetUniform( int uniformLocation, int value )
{
	glUniform1i(uniformLocation,value);
}


void OpenGLRenderer::SetUniform( int uniformLocation, float value )
{
	glUniform1f(uniformLocation,value);
}


void OpenGLRenderer::SetUniform( int uniformLocation, Matrix4 matrix)
{
	glUniformMatrix4fv( uniformLocation , 1 , false , matrix.m_matrix );
}


void OpenGLRenderer::SetUniform(int uniformLocation, Vec3f vector3)
{
	float vec[3] = {vector3.x,vector3.y,vector3.z};
	glUniform3fv( uniformLocation , 1 , vec );
}


void OpenGLRenderer::SetUniform(int uniformLocation, RGBA color)
{
	const float oneOver255 = 0.003921f;
	float colorF[4] = {color.r * oneOver255, color.g * oneOver255 , color.b * oneOver255 , color.a * oneOver255};
	glUniform4fv( uniformLocation , 1 , colorF );
}


void OpenGLRenderer::SetUniform(int uniformLocation, std::vector<RGBA> colors)
{
	std::vector<float*> fcolors;
	fcolors.reserve(colors.size() + 1);
	const float oneOver255 = 0.003921f;
	for (size_t index = 0; index < colors.size(); ++index)
	{
		float color[4];
		color[0] = (int)colors[index].r * oneOver255;
		color[1] = (int)colors[index].g * oneOver255;
		color[2] = (int)colors[index].b * oneOver255;
		color[3] = (int)colors[index].a * oneOver255;
		fcolors.push_back(color);
		//printf("%d. %f , %f , %f , %f\n", index, color[0], color[1], color[2], color[3]);
	}

	glUniform4fv( uniformLocation , colors.size() , fcolors[0] );
}

void OpenGLRenderer::SetUniform(int uniformLocation, std::vector<Vec2f> vector2)
{
	glUniform2fv( uniformLocation , vector2.size() , &vector2[0].x );
}


void OpenGLRenderer::SetUniform( int uniformLocation, std::vector<Vec3f> vector3)
{
	glUniform3fv( uniformLocation , vector3.size() , &vector3[0].x );
}


void OpenGLRenderer::SetUniform( int uniformLocation, std::vector<float> value)
{
	glUniform1fv( uniformLocation , value.size() , &value[0] );
}


void OpenGLRenderer::DrawElements( std::vector<GLfloat> vertices , std::vector<GLfloat> normals ,  std::vector<GLfloat> texCoords , std::vector<GLshort> indices ,  GLenum drawMode)
{
	glEnable(GL_TEXTURE_2D);

	int vertexArrayID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_position");
	int texArrayID = glGetAttribLocation(SHADER_PROGRAM_ID,"a_textureCoords");

	glEnableVertexAttribArray(vertexArrayID);
	glEnableVertexAttribArray(texArrayID);

	glVertexAttribPointer(vertexArrayID,3,GL_FLOAT,false,0, &vertices[0]);
	glVertexAttribPointer(texArrayID,2,GL_FLOAT,false,0, &texCoords[0]);

	glDrawElements(drawMode, indices.size(), GL_UNSIGNED_SHORT, &indices[0]);

	glDisableVertexAttribArray(vertexArrayID);
	glDisableVertexAttribArray(texArrayID);
	
	glDisable(GL_TEXTURE_2D);
}


void OpenGLRenderer::DrawVertexWithVertexArray2D(Vertex_PosColor* vertices,int sizeOfVertices,GLenum glDrawMode,float width,float height)
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	PushMatrix();
	LoadIdentity();
	SetOrtho( 0.0 , width , 0.0 , height , 0 , 1 );

	DrawVertexWithVertexArray(vertices,sizeOfVertices,glDrawMode);

	PopMatrix();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
}


void OpenGLRenderer::DrawVertexWithVertexArray2D(Vertex_PCT* vertices,int sizeOfVertices,GLenum glDrawMode,float width,float height)
{
	//UseProgram(SHADER_PROGRAM_ID);
	glDisable(GL_DEPTH_TEST);
	PushMatrix();
	LoadIdentity();
	SetOrtho( 0.0 , width , 0.0 , height , 0 , 1 );

	DrawVertexWithVertexArray(vertices,sizeOfVertices,glDrawMode);

	PopMatrix();
	glEnable(GL_DEPTH_TEST);
}


void OpenGLRenderer::DrawVertexWithVertexArray2D(std::vector<Vertex_PCT>& vertices,GLenum glDrawMode,float width,float height)
{
	if(vertices.size() == 0)
		return;

	//UseProgram(SHADER_PROGRAM_ID);
	glDisable(GL_DEPTH_TEST);
	PushMatrix();
	LoadIdentity();
	SetOrtho( 0.0 , width , 0.0 , height , 0 , 1 );

	DrawVertexWithVertexArray(vertices,glDrawMode);

	PopMatrix();
	glEnable(GL_DEPTH_TEST);
}


void OpenGLRenderer::DrawVertexWithVertexArray2D(std::vector<Vertex_PosColor>& vertices, GLenum glDrawMode, float width, float height)
{
	if (vertices.size() == 0)
		return;

	//UseProgram(SHADER_PROGRAM_ID);
	glDisable(GL_DEPTH_TEST);
	PushMatrix();
	LoadIdentity();
	SetOrtho(0.0, width, 0.0, height, 0, 1);

	DrawVertexWithVertexArray(vertices, glDrawMode);

	PopMatrix();
	glEnable(GL_DEPTH_TEST);
}


void OpenGLRenderer::PushMatrix()
{
	glPushMatrix();
	if(WorldToClipMatrixStack.size() == 0)
	{
		WorldToClipMatrixStack.push_back(Matrix4());
		ObjectToWorldMatrixStack.push_back(Matrix4());
	}
	else
	{
		WorldToClipMatrixStack.push_back(WorldToClipMatrixStack.back());
		ObjectToWorldMatrixStack.push_back(ObjectToWorldMatrixStack.back());
	}
}


void OpenGLRenderer::PopMatrix()
{
	glPopMatrix();
	if(WorldToClipMatrixStack.size() != 1 && ObjectToWorldMatrixStack.size() != 1)
	{
		WorldToClipMatrixStack.pop_back();
		ObjectToWorldMatrixStack.pop_back();
	}
}


void OpenGLRenderer::Scalef(float x, float y, float z)
{
	glScalef(x,y,z);
	Matrix4& top = WorldToClipMatrixStack.back();

	top.m_matrix[0] *= x;   top.m_matrix[4] *= x;   top.m_matrix[8] *= x;   top.m_matrix[12] *= x;
	top.m_matrix[1] *= y;   top.m_matrix[5] *= y;   top.m_matrix[9] *= y;   top.m_matrix[13] *= y;
	top.m_matrix[2] *= z;   top.m_matrix[6] *= z;   top.m_matrix[10]*= z;   top.m_matrix[14] *= z;
}


void OpenGLRenderer::Translatef(float x, float y, float z)
{
	glTranslatef(x,y,z);

	Matrix4& top = WorldToClipMatrixStack.back();
	/*
	Matrix4 trans;
	trans.m_matrix[12] = x;
	trans.m_matrix[13] = y;
	trans.m_matrix[14] = z;
	top.ApplyTransformMatrix(trans.m_matrix);*/
	
	top.m_matrix[12] += top.m_matrix[0] * x + top.m_matrix[4] * y + top.m_matrix[8]  * z;
	top.m_matrix[13] += top.m_matrix[1] * x + top.m_matrix[5] * y + top.m_matrix[9]  * z;
	top.m_matrix[14] += top.m_matrix[2] * x + top.m_matrix[6] * y + top.m_matrix[10] * z;
	top.m_matrix[15] += top.m_matrix[3] * x + top.m_matrix[7] * y + top.m_matrix[11] * z;
}


void OpenGLRenderer::Rotatef(float degrees,float x,float y,float z)
{
	glRotatef(degrees,x,y,z);
	Matrix4& top = WorldToClipMatrixStack.back();
	float radians = degree2radians(degrees);
	float _cos = cos(radians);
	float _sin = sin(radians);

	Vec3f axis = Vec3f(x,y,z);
	axis.normalize();

	float c1 = 1.0f - _cos;
	float rotate[16];
	// build rotation matrix
	rotate[0] = axis.x * axis.x * c1 + _cos;
	rotate[1] = axis.x * axis.y * c1 + axis.z * _sin;
	rotate[2] = axis.x * axis.z * c1 - axis.y * _sin;
	rotate[3] = 0.0f;
	rotate[4] = axis.y * axis.x * c1 - axis.z * _sin;
	rotate[5] = axis.y * axis.y * c1 + _cos;
	rotate[6] = axis.y * axis.z * c1 + axis.x * _sin;
	rotate[7] = 0.0f;
	rotate[8] = axis.z * axis.x * c1 + axis.y * _sin;
	rotate[9] = axis.z * axis.y * c1 - axis.x * _sin;
	rotate[10]= axis.z * axis.z * c1 + _cos;
	rotate[11] = 0.0f;
	rotate[12] = 0.0f;
	rotate[13] = 0.0f;
	rotate[14] = 0.0f;
	rotate[15] = 1.0f;

	top.ApplyTransformMatrix(rotate);
}


void OpenGLRenderer::LoadIdentity()
{
	glLoadIdentity();
	Matrix4& top = WorldToClipMatrixStack.back();
	top.m_matrix[0] = 1.0f;	top.m_matrix[4] = 0.0f;	top.m_matrix[8] = 0.0f;		top.m_matrix[12] = 0.0f;
	top.m_matrix[1] = 0.0f;	top.m_matrix[5] = 1.0f;	top.m_matrix[9] = 0.0f;		top.m_matrix[13] = 0.0f;
	top.m_matrix[2] = 0.0f;	top.m_matrix[6] = 0.0f;	top.m_matrix[10] = 1.0f;	top.m_matrix[14] = 0.0f;
	top.m_matrix[3] = 0.0f;	top.m_matrix[7] = 0.0f;	top.m_matrix[11] = 0.0f;	top.m_matrix[15] = 1.0f;
}


Matrix4 OpenGLRenderer::Perspective(float fov, float aspect, float _near, float _far)
{
	gluPerspective(fov,aspect,_near,_far);
	float radians = degree2radians(fov * 0.5f);
	float tangent = tanf(radians);
	float height = _near * tangent;         
	float width = height * aspect;
	return SetFrustum(-width, width, -height, height, _near, _far);
}


Matrix4 OpenGLRenderer::SetFrustum(float l, float r, float b, float t, float n, float f)
{
	Matrix4& matrix = WorldToClipMatrixStack.back();
	matrix.m_matrix[0]  = 2 * n / (r - l);
	matrix.m_matrix[5]  = 2 * n / (t - b);
	matrix.m_matrix[8]  = (r + l) / (r - l);
	matrix.m_matrix[9]  = (t + b) / (t - b);
	matrix.m_matrix[10] = -(f + n) / (f - n);
	matrix.m_matrix[11] = -1;
	matrix.m_matrix[14] = -(2 * f * n) / (f - n);
	matrix.m_matrix[15] = 0;

	return matrix;
}


Matrix4 OpenGLRenderer::SetOrtho(float l, float r, float b, float t , float n, float f)
{
	LoadIdentity();
	//glOrtho(l,r,b,t,n,f);
	Matrix4& top = WorldToClipMatrixStack.back();
	top.m_matrix[0]  = 2 / (r - l);
	top.m_matrix[5]  = 2 / (t - b);
	top.m_matrix[10] = -2 / (f - n);
	top.m_matrix[12] = -(r + l) / (r - l);
	top.m_matrix[13] = -(t + b) / (t - b);
	top.m_matrix[14] = -(f + n) / (f - n);

	return top;
}


void OpenGLRenderer::DrawPoint3D(Vec3f position,float size,RGBA color)
{
	Vertex_PosColor vertices[8];
	vertices[0].position = Vec3f(position.x - size , position.y - size , position.z - size);
	vertices[1].position = Vec3f(position.x + size , position.y + size , position.z + size);
	vertices[2].position = Vec3f(position.x - size , position.y - size , position.z + size);
	vertices[3].position = Vec3f(position.x + size , position.y + size , position.z - size);
	vertices[4].position = Vec3f(position.x + size , position.y - size , position.z + size);
	vertices[5].position = Vec3f(position.x - size , position.y + size , position.z - size);
	vertices[6].position = Vec3f(position.x - size , position.y + size , position.z + size);
	vertices[7].position = Vec3f(position.x + size , position.y - size , position.z - size);

	vertices[0].color = color;
	vertices[1].color = color;
	vertices[2].color = color;
	vertices[3].color = color;
	vertices[4].color = color;
	vertices[5].color = color;
	vertices[6].color = color;
	vertices[7].color = color;

	glLineWidth(4.5f);
	DrawVertexWithVertexArray(vertices , 8 , GL_LINES );
	DepthTest(true);

	for(int index=0; index<8; index++)
		vertices[index].color *= 0.5f;

	glLineWidth(1.0f);
	DepthTest(false);
	DrawVertexWithVertexArray(vertices , 8 , GL_LINES );
	DepthTest(true);
}


void OpenGLRenderer::InitializeFrameBuffer(FBO& fbo, Vec2i textureSize)
{
	GLuint frameBufferObjectID = 0;
	GLuint frameBufferColorTextureID = 0;
	GLuint frameBufferDepthTextureID = 0;

	// Create color framebuffer texture
	glActiveTexture( GL_TEXTURE0 );
	frameBufferColorTextureID = CreateTexture(textureSize.x, textureSize.y);

	// Create depth framebuffer texture
	glActiveTexture( GL_TEXTURE1 );
	frameBufferDepthTextureID = CreateTexture(textureSize.x, textureSize.y, true);

	// Create an FBO (Framebuffer Object) and activate it
	glGenFramebuffers( 1, &frameBufferObjectID );
	glBindFramebuffer( GL_FRAMEBUFFER, frameBufferObjectID );

	// Attach our color and depth textures to the FBO, in the color0 and depth FBO "slots"
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBufferColorTextureID, 0 );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, frameBufferDepthTextureID, 0);

	fbo.ID = frameBufferObjectID;
	fbo.colorTextureID = frameBufferColorTextureID;
	fbo.depthTextureID = frameBufferDepthTextureID;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}						 


void OpenGLRenderer::SendBasicTwoFBOTextures(int framebufferColorTextureID , int framebufferDepthTextureID)
{
	glClearDepth( 1.f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glDisable( GL_DEPTH_TEST );
	glDepthMask( GL_FALSE );

	glActiveTexture( GL_TEXTURE1 );
	glEnable(GL_TEXTURE_2D);
	glBindTexture( GL_TEXTURE_2D, framebufferDepthTextureID );
	BindTexture(framebufferDepthTextureID,1);

	glActiveTexture( GL_TEXTURE0 );
	glEnable(GL_TEXTURE_2D);
	glBindTexture( GL_TEXTURE_2D, framebufferColorTextureID );
	BindTexture(framebufferColorTextureID,0);

	int currentMatrixLocation = OpenGLRenderer::GetUniformLocation( SHADER_PROGRAM_ID , "u_worldToClipMatrix" );
	SetUniform( currentMatrixLocation , WorldToClipMatrixStack.back() );

	int colorMapUniformLocation = OpenGLRenderer::GetUniformLocation( SHADER_PROGRAM_ID , "u_colorMap" );
	SetUniform( colorMapUniformLocation , 0 );

	int depthMapUniformLocation = OpenGLRenderer::GetUniformLocation( SHADER_PROGRAM_ID , "u_depthMap" );
	SetUniform( depthMapUniformLocation , 1 );

	int textureWidth = OpenGLRenderer::GetUniformLocation( SHADER_PROGRAM_ID , "u_textureWidth" );
	SetUniform( textureWidth , WINDOW_SIZE.x );

	int textureHeight = OpenGLRenderer::GetUniformLocation( SHADER_PROGRAM_ID , "u_textureHeight" );
	SetUniform( textureHeight , WINDOW_SIZE.y );

	glEnable( GL_DEPTH_TEST );
	glDepthMask( GL_TRUE );
}


int OpenGLRenderer::CreateTexture( int width, int height, bool isDepth /* = false */ )
{
	unsigned int textureId;
	glGenTextures(1,&textureId);
	glBindTexture(GL_TEXTURE_2D,textureId);
	glTexImage2D(GL_TEXTURE_2D,0,(!isDepth ? GL_RGBA8 : GL_DEPTH_COMPONENT),width,height,0,isDepth ? GL_DEPTH_COMPONENT : GL_RGBA,GL_FLOAT,NULL);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

	if( glGetError() != 0)
	{
		MessageBoxA(nullptr,"Error happened while creating the texture", "Fail to Create Texture", MB_ICONERROR | MB_OK);
	}
	glBindTexture(GL_TEXTURE_2D , 0);
	return textureId;
}


int OpenGLRenderer::CreateDefaultDiffuseTexture()
{
	int openglTextureID = 0;
	unsigned char* imageData = new unsigned char[4];
	imageData[0] = 255;
	imageData[1] = 255;
	imageData[2] = 255;
	imageData[3] = 255;

	// Enable texturing
	glEnable( GL_TEXTURE_2D );

	// Tell OpenGL that our pixel data is single-byte aligned
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	// Ask OpenGL for an unused texName (ID number) to use for this texture
	glGenTextures( 1, (GLuint*) &openglTextureID );

	// Tell OpenGL to bind (set) this as the currently active texture
	glBindTexture( GL_TEXTURE_2D, openglTextureID );

	// Set texture clamp vs. wrap (repeat)
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT ); // one of: GL_CLAMP_TO_EDGE, GL_REPEAT, GL_MIRRORED_REPEAT, GL_MIRROR_CLAMP_TO_EDGE, ...
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ); // one of: GL_CLAMP_TO_EDGE, GL_REPEAT, GL_MIRRORED_REPEAT, GL_MIRROR_CLAMP_TO_EDGE, ...

	// Set magnification (texel > pixel) and minification (texel < pixel) filters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); // one of: GL_NEAREST, GL_LINEAR
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ); // one of: GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR

	GLenum bufferFormat = GL_RGBA; // the format our source pixel data is currently in; any of: GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, ...
	GLenum internalFormat = bufferFormat; // the format we want the texture to me on the card; allows us to translate into a different texture format as we upload to OpenGL

	glTexImage2D(			// Upload this pixel data to our new OpenGL texture
		GL_TEXTURE_2D,		// Creating this as a 2d texture
		0,					// Which mipmap level to use as the "root" (0 = the highest-quality, full-res image), if mipmaps are enabled
		internalFormat,		// Type of texel format we want OpenGL to use for this texture internally on the video card
		1,					// Texel-width of image; for maximum compatibility, use 2^N + 2^B, where N is some integer in the range [3,10], and B is the border thickness [0,1]
		1,					// Texel-height of image; for maximum compatibility, use 2^M + 2^B, where M is some integer in the range [3,10], and B is the border thickness [0,1]
		0,					// Border size, in texels (must be 0 or 1)
		bufferFormat,		// Pixel format describing the composition of the pixel data in buffer
		GL_UNSIGNED_BYTE,	// Pixel color components are unsigned bytes (one byte per color/alpha channel)
		imageData );		// Location of the actual pixel data bytes/buffer

	delete imageData;
	return openglTextureID;
}


int OpenGLRenderer::CreateDefaultNormalTexture()
{
	int openglTextureID = 0;
	unsigned char* imageData = new unsigned char[4];
	imageData[0] = 128;
	imageData[1] = 128;
	imageData[2] = 255;
	imageData[3] = 255;

	// Enable texturing
	glEnable(GL_TEXTURE_2D);

	// Tell OpenGL that our pixel data is single-byte aligned
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Ask OpenGL for an unused texName (ID number) to use for this texture
	glGenTextures(1, (GLuint*)&openglTextureID);

	// Tell OpenGL to bind (set) this as the currently active texture
	glBindTexture(GL_TEXTURE_2D, openglTextureID);

	// Set texture clamp vs. wrap (repeat)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // one of: GL_CLAMP_TO_EDGE, GL_REPEAT, GL_MIRRORED_REPEAT, GL_MIRROR_CLAMP_TO_EDGE, ...
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // one of: GL_CLAMP_TO_EDGE, GL_REPEAT, GL_MIRRORED_REPEAT, GL_MIRROR_CLAMP_TO_EDGE, ...

	// Set magnification (texel > pixel) and minification (texel < pixel) filters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // one of: GL_NEAREST, GL_LINEAR
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // one of: GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR

	GLenum bufferFormat = GL_RGBA; // the format our source pixel data is currently in; any of: GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, ...
	GLenum internalFormat = bufferFormat; // the format we want the texture to me on the card; allows us to translate into a different texture format as we upload to OpenGL

	glTexImage2D(			// Upload this pixel data to our new OpenGL texture
		GL_TEXTURE_2D,		// Creating this as a 2d texture
		0,					// Which mipmap level to use as the "root" (0 = the highest-quality, full-res image), if mipmaps are enabled
		internalFormat,		// Type of texel format we want OpenGL to use for this texture internally on the video card
		1,					// Texel-width of image; for maximum compatibility, use 2^N + 2^B, where N is some integer in the range [3,10], and B is the border thickness [0,1]
		1,					// Texel-height of image; for maximum compatibility, use 2^M + 2^B, where M is some integer in the range [3,10], and B is the border thickness [0,1]
		0,					// Border size, in texels (must be 0 or 1)
		bufferFormat,		// Pixel format describing the composition of the pixel data in buffer
		GL_UNSIGNED_BYTE,	// Pixel color components are unsigned bytes (one byte per color/alpha channel)
		imageData);		// Location of the actual pixel data bytes/buffer

	delete imageData;
	return openglTextureID;
}


int OpenGLRenderer::CreateDefaultSpecularTexture()
{
	int openglTextureID = 0;
	unsigned char* imageData = new unsigned char[4];
	imageData[0] = 128;
	imageData[1] = 128;
	imageData[2] = 0;
	imageData[3] = 255;

	// Enable texturing
	glEnable( GL_TEXTURE_2D );

	// Tell OpenGL that our pixel data is single-byte aligned
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	// Ask OpenGL for an unused texName (ID number) to use for this texture
	glGenTextures( 1, (GLuint*) &openglTextureID );

	// Tell OpenGL to bind (set) this as the currently active texture
	glBindTexture( GL_TEXTURE_2D, openglTextureID );

	// Set texture clamp vs. wrap (repeat)
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT ); // one of: GL_CLAMP_TO_EDGE, GL_REPEAT, GL_MIRRORED_REPEAT, GL_MIRROR_CLAMP_TO_EDGE, ...
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ); // one of: GL_CLAMP_TO_EDGE, GL_REPEAT, GL_MIRRORED_REPEAT, GL_MIRROR_CLAMP_TO_EDGE, ...

	// Set magnification (texel > pixel) and minification (texel < pixel) filters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); // one of: GL_NEAREST, GL_LINEAR
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ); // one of: GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR

	GLenum bufferFormat = GL_RGBA; // the format our source pixel data is currently in; any of: GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, ...
	GLenum internalFormat = bufferFormat; // the format we want the texture to me on the card; allows us to translate into a different texture format as we upload to OpenGL

	glTexImage2D(			// Upload this pixel data to our new OpenGL texture
		GL_TEXTURE_2D,		// Creating this as a 2d texture
		0,					// Which mipmap level to use as the "root" (0 = the highest-quality, full-res image), if mipmaps are enabled
		internalFormat,		// Type of texel format we want OpenGL to use for this texture internally on the video card
		1,					// Texel-width of image; for maximum compatibility, use 2^N + 2^B, where N is some integer in the range [3,10], and B is the border thickness [0,1]
		1,					// Texel-height of image; for maximum compatibility, use 2^M + 2^B, where M is some integer in the range [3,10], and B is the border thickness [0,1]
		0,					// Border size, in texels (must be 0 or 1)
		bufferFormat,		// Pixel format describing the composition of the pixel data in buffer
		GL_UNSIGNED_BYTE,	// Pixel color components are unsigned bytes (one byte per color/alpha channel)
		imageData );		// Location of the actual pixel data bytes/buffer

	delete imageData;
	return openglTextureID;
}


void OpenGLRenderer::GenerateMipMap(int level)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, level);
	glGenerateMipmap(GL_TEXTURE_2D);
}


void OpenGLRenderer::PointSize(float size)
{
	glPointSize(size);
}


void OpenGLRenderer::DeleteBuffer(GLuint id)
{
	glDeleteBuffers(1,&id);
}


void OpenGLRenderer::GenerateBuffers(int* bufferID)
{
	GLuint id;
	glGenBuffers( 1 , &id );
	*bufferID = id;
}


void OpenGLRenderer::BufferData(std::vector<Vertex_PosColor>& vertices,int vboID)
{
	glBindBuffer( GL_ARRAY_BUFFER, vboID );
	glBufferData( GL_ARRAY_BUFFER, sizeof( Vertex_PosColor ) * vertices.size(), vertices.data(), GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
}


void OpenGLRenderer::BufferData(Vertex_PosColor* vertices,int verticesSize,int vboID)
{
	glBindBuffer( GL_ARRAY_BUFFER, vboID );
	glBufferData( GL_ARRAY_BUFFER, sizeof( Vertex_PosColor ) * verticesSize, &vertices[0], GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
}


void OpenGLRenderer::BufferData(std::vector<Vertex_PCTN>& vertices,int vboID)
{
	glBindBuffer( GL_ARRAY_BUFFER, vboID );
	glBufferData( GL_ARRAY_BUFFER, sizeof( Vertex_PCTN ) * vertices.size(), vertices.data(), GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
}


void OpenGLRenderer::BufferData(Vertex_PCTN* vertices,int verticesSize,int vboID)
{
	glBindBuffer( GL_ARRAY_BUFFER, vboID );
	glBufferData( GL_ARRAY_BUFFER, sizeof( Vertex_PCTN ) * verticesSize, &vertices[0], GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
}


void OpenGLRenderer::BufferData(std::vector<int>& indices,int iboID)
{
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, iboID );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( int ) * indices.size(), indices.data(), GL_STATIC_DRAW );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
}


void OpenGLRenderer::GetImageData(int textureID, unsigned char*& data)
{
	GLint whichID;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &whichID);

	BindTexture(textureID);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	BindTexture(whichID);
}


int OpenGLRenderer::GetTextureID()
{
	int currentTextureID;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &currentTextureID);
	return currentTextureID;
}


};