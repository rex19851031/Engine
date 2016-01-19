#pragma once

#ifndef OPENGLRENDERER_HPP
#define OPENGLRENDERER_HPP

#define STBI_HEADER_FILE_ONLY
#include "stb_image.c"

#define WIN32_LEAN_AND_MEAN
#include <string>
#include <vector>
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <Xinput.h>					// include the Xinput API
#pragma comment( lib, "xinput" )	// Link in the xinput.lib static library
#pragma comment( lib, "glu32" ) // Link in the OpenGL32.lib static library
#pragma comment( lib, "opengl32" ) // Link in the OpenGL32.lib static library

#include "Engine\Core\VertexStruct.hpp"
#include "Engine\Renderer\Texture.hpp"
#include "Engine\Math\Matrix4.hpp"
#include "Engine\Math\GeneralStruct.hpp"

namespace Henry
{

class OpenGLRenderer
{
public:
	static void BlendMode( GLenum blendSource, GLenum blendDestination );

	static void Initialize(Vec2i windowSize);

	static void InitializeFrameBuffer(FBO& fbo, Vec2i textureSize = WINDOW_SIZE);

	static void ClearScreen( float r , float g , float b );

	static void ChangeTexture(int TextureID);

	static int CreateTexture( int width, int height, bool isDepth = false );

	static int LoadTexture( const std::string& imageFilePath , Vec2i& size );

	static int LoadTexture( const std::string& imageFilePath , Vec2i& size , const std::string& zipFilePath );

	static int LoadTextureFromData( const unsigned char* imageData , Vec2i& size );

	static void BindTexture(Texture* diffuse = nullptr, Texture* emissive = nullptr, Texture* normal = nullptr, Texture* specular = nullptr);

	static void BindTexture(int textureID, int textureUnit = 0);

	static void BindFrameBuffer(int bufferID);

	static void FrameBufferColorTexture(int textureID, int textureUnit = 0);

	static void DrawVertexWithVertexArray(Vertex_PosColor* vertices,int sizeOfVertices,GLenum glDrawMode);

	static void DrawVertexWithVertexArray(Vertex_PCT* vertices,int sizeOfVertices,GLenum glDrawMode);
	
	static void DrawVertexWithVertexArray(std::vector<Vertex_PCTN>& vertices,GLenum glDrawMode);

	static void DrawVertexWithVertexArray(std::vector<Vertex_PCT>& vertices,GLenum glDrawMode);

	static void DrawVertexWithVertexArray(std::vector<Vertex_PosColor>& vertices,GLenum glDrawMode);

	static void DrawVertexWithVertexArray(Vertex_PCTN* vertices,int sizeOfVertices,GLenum glDrawMode);

	static void DrawVertexWithVertexBufferObject(int vboID , int numOfVertices , GLenum glDrawMode , Vertex_PosColor);

	static void DrawVertexWithVertexBufferObject(int vboID , int numOfVertices , GLenum glDrawMode , Vertex_PCTN);

	static void DrawVertexWithIndexedVertexBufferObject(int vboID , int iboID , int numOfVertices , int numOfIndices , GLenum glDrawMode , Vertex_PCTN);

	static void DrawVertexWithVertexArray2D(Vertex_PosColor* vertices,int sizeOfVertices,GLenum glDrawMode,float width = WINDOW_SIZE.x , float height = WINDOW_SIZE.y );

	static void DrawVertexWithVertexArray2D(Vertex_PCT* vertices,int sizeOfVertices,GLenum glDrawMode,float width = WINDOW_SIZE.x , float height = WINDOW_SIZE.y );

	static void DrawVertexWithVertexArray2D(std::vector<Vertex_PCT>& vertices,GLenum glDrawMode,float width = WINDOW_SIZE.x , float height = WINDOW_SIZE.y);

	static void DrawVertexWithVertexArray2D(std::vector<Vertex_PosColor>& vertices, GLenum glDrawMode, float width = WINDOW_SIZE.x , float height = WINDOW_SIZE.y);

	static void DrawSolidSphere(Vec3f center,float radius, RGBA color ,int slice,int stacks);
	
	static void DrawWireSphere(Vec3f center,float radius, RGBA color ,int slices,int stacks);

	static void DrawPoint3D(Vec3f position,float size,RGBA color);

	static void DeleteBuffer(GLuint id);

	static void DepthTest(bool value);

	static void PointSmooth(bool value);

	static void LineWidth(float width);

	static GLuint CreateProgram();

	static void AttachShader(GLuint programID,GLuint shaderID);

	static void LinkProgram(GLuint programID);

	static void GetProgramIV(GLuint programID,GLenum name,bool &wasSuccessful);

	static void UseProgram(int programID);

	static std::string GetString(GLenum variable);

	static GLint GetUniformLocation(int programID,const char* variableName);
	
	static void SetUniform( int uniformLocation , Vec2f value);
	
	static void SetUniform( int uniformLocation, float value );

	static void SetUniform( int uniformLocation, int value );

	static void SetUniform( int uniformLocation, Matrix4 matrix);

	static void SetUniform( int uniformLocation, Vec3f vector3);

	static void SetUniform( int uniformLocation, RGBA color);

	static void SetUniform( int uniformLocation, std::vector<RGBA> color);

	static void SetUniform( int uniformLocation, std::vector<Vec3f> vector3);

	static void SetUniform( int uniformLocation, std::vector<float> value);

	static void SetUniform( int uniformLocation, std::vector<Vec2f> vector2);

	static void DrawElements(  std::vector<GLfloat> vertices , std::vector<GLfloat> normals ,  std::vector<GLfloat> textCoords , std::vector<GLshort> indices ,  GLenum drawMode );

	static void PushMatrix();

	static void PopMatrix();
	
	static void Scalef(float x, float y, float z);
	
	static void Translatef(float x, float y, float z);
	
	static void Rotatef(float degrees, float x, float y, float z);

	static void LoadIdentity();

	static Matrix4 Perspective(float fov,float aspect,float near,float far);

	static Matrix4 SetFrustum(float l, float r, float b, float t, float n, float f);

	static Matrix4 SetOrtho(float l = 0.0f, float r = WINDOW_SIZE.x, float b = 0.0f, float t = WINDOW_SIZE.y, float n = 0.0f, float f = 1.0f);

	static void SendBasicTwoFBOTextures(int framebufferColorTextureID , int framebufferDepthTextureID);

	static int CreateDefaultDiffuseTexture();

	static int CreateDefaultNormalTexture();

	static int CreateDefaultSpecularTexture();

	static int CreateBlankTexture(const Vec2i& size);

	static void BindWhiteTexture();

	static void GenerateMipMap(int level);

	static void GenerateBuffers(int* bufferID);

	static void BufferData(std::vector<Vertex_PosColor>& vertices,int vboID);
	
	static void BufferData(Vertex_PosColor* vertices,int verticesSize,int vboID);

	static void BufferData(std::vector<Vertex_PCTN>& vertices,int vboID);
	
	static void BufferData(Vertex_PCTN* vertices,int verticesSize,int vboID);

	static void BufferData(std::vector<int>& indices,int iboID);

	static void PointSize(float size);

	static void GetImageData(int textureID, unsigned char*& data);

	static int GetFrameBufferID();

	static int GetTextureID();

	static GLenum ONE_MINUS_DESTINATION_COLOR;
	
	static GLenum ONE;

	static GLenum ZERO;

	static GLenum SRC_ALPHA;
	
	static GLenum ONE_MINUS_SRC_ALPHA;

	static GLenum VERTEX_SHADER;

	static GLenum FRAGMENT_SHADER;

	static GLenum LINK_STATUS;

	static GLenum SHAPE_QUADS;

	static GLenum VERSION;

	static GLenum SHADING_LANGUAGE_VERSION;

	static GLenum SHAPE_LINES;

	static GLenum SHAPE_POINTS;

	static GLenum SHAPE_TRIANGLES;

	static int SHADER_PROGRAM_ID;

	static std::vector<Matrix4> WorldToClipMatrixStack;

	static std::vector<Matrix4> ObjectToWorldMatrixStack;

	static int DIFFUSE_TEXTURE_ID;

	static int NORMAL_TEXTURE_ID;

	static int SPECULAR_TEXTURE_ID;

	static Vec2i WINDOW_SIZE;
};

};

#endif