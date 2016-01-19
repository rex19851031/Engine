#include "Engine\Core\DebugDrawShapes.hpp"
#include "Engine\Renderer\OpenGLRenderer.hpp"

namespace Henry
{

void DebugDrawPosition::draw()
{
	Vertex_PosColor vertices[8];
	vertices[0].position = Vec3f(m_position.x - m_radius , m_position.y - m_radius , m_position.z - m_radius);
	vertices[1].position = Vec3f(m_position.x + m_radius , m_position.y + m_radius , m_position.z + m_radius);
	vertices[2].position = Vec3f(m_position.x - m_radius , m_position.y - m_radius , m_position.z + m_radius);
	vertices[3].position = Vec3f(m_position.x + m_radius , m_position.y + m_radius , m_position.z - m_radius);
	vertices[4].position = Vec3f(m_position.x + m_radius , m_position.y - m_radius , m_position.z + m_radius);
	vertices[5].position = Vec3f(m_position.x - m_radius , m_position.y + m_radius , m_position.z - m_radius);
	vertices[6].position = Vec3f(m_position.x - m_radius , m_position.y + m_radius , m_position.z + m_radius);
	vertices[7].position = Vec3f(m_position.x + m_radius , m_position.y - m_radius , m_position.z - m_radius);

	vertices[0].color = m_color;
	vertices[1].color = m_color;
	vertices[2].color = m_color;
	vertices[3].color = m_color;
	vertices[4].color = m_color;
	vertices[5].color = m_color;
	vertices[6].color = m_color;
	vertices[7].color = m_color;

	if(g_debugMode == DEPTH_TEST_OFF)
		OpenGLRenderer::DepthTest(false);

	if(g_debugMode == DEPTH_TEST_ON)
		OpenGLRenderer::DepthTest(true);

	glLineWidth(4.5f);
	OpenGLRenderer::DrawVertexWithVertexArray(vertices , 8 , GL_LINES );

	OpenGLRenderer::DepthTest(true);

	if(g_debugMode != DUAL_MODE)
		return;

	for(int index=0; index<8; index++)
		vertices[index].color *= 0.5f;

	glLineWidth(1.0f);
	OpenGLRenderer::DepthTest(false);
	OpenGLRenderer::DrawVertexWithVertexArray(vertices , 8 , GL_LINES );
	OpenGLRenderer::DepthTest(true);
}


void DebugDrawLine::draw()
{
	Vertex_PosColor vertices[2];
	vertices[0].position = m_startPos;
	vertices[1].position = m_endPos;
	
	vertices[0].color = m_startColor;
	vertices[1].color = m_endColor;
	
	if(g_debugMode == DEPTH_TEST_OFF)
		OpenGLRenderer::DepthTest(false);

	if(g_debugMode == DEPTH_TEST_ON)
		OpenGLRenderer::DepthTest(true);

	glLineWidth(4.5f);
	OpenGLRenderer::DrawVertexWithVertexArray(vertices , 2 , GL_LINES );

	OpenGLRenderer::DepthTest(true);

	if(g_debugMode != DUAL_MODE)
		return;

	for(int index=0; index<2; index++)
		vertices[index].color *= 0.5f;

	glLineWidth(1.0f);
	OpenGLRenderer::DepthTest(false);
	OpenGLRenderer::DrawVertexWithVertexArray(vertices , 2 , GL_LINES);
	OpenGLRenderer::DepthTest(true);
}


void DebugDrawArrow::draw()
{
	Vertex_PosColor vertices[2];
	vertices[0].position = m_startPos;
	vertices[1].position = m_endPos;

	vertices[0].color = m_startColor;
	vertices[1].color = m_endColor;

	if(g_debugMode == DEPTH_TEST_OFF)
		OpenGLRenderer::DepthTest(false);

	if(g_debugMode == DEPTH_TEST_ON)
		OpenGLRenderer::DepthTest(true);

	glLineWidth(4.5f);
	OpenGLRenderer::DrawVertexWithVertexArray(vertices , 2 , GL_LINES );
	
	DebugDrawPosition ddp(m_endPos,m_endColor,0.1f);
	ddp.draw();

	OpenGLRenderer::DepthTest(true);

	if(g_debugMode != DUAL_MODE)
		return;

	for(int index=0; index<2; index++)
		vertices[index].color *= 0.5f;

	glLineWidth(1.0f);
	OpenGLRenderer::DepthTest(false);
	OpenGLRenderer::DrawVertexWithVertexArray(vertices , 2 , GL_LINES );
	OpenGLRenderer::DepthTest(true);
}


void DebugDrawAABB::draw()
{
	Vertex_PosColor vertices[24];
	vertices[0].position =	Vec3f(m_minPos.x , m_minPos.y , m_minPos.z );
	vertices[1].position =	Vec3f(m_minPos.x , m_maxPos.y , m_minPos.z );
	vertices[2].position =	Vec3f(m_maxPos.x , m_maxPos.y , m_minPos.z );
	vertices[3].position =	Vec3f(m_maxPos.x , m_minPos.y , m_minPos.z );
											   		    			 
	vertices[4].position =	Vec3f(m_maxPos.x , m_minPos.y , m_minPos.z );
	vertices[5].position =	Vec3f(m_maxPos.x , m_maxPos.y , m_minPos.z );
	vertices[6].position =	Vec3f(m_maxPos.x , m_maxPos.y , m_maxPos.z );
	vertices[7].position =	Vec3f(m_maxPos.x , m_minPos.y , m_maxPos.z );
											   		   				 
	vertices[8].position =	Vec3f(m_maxPos.x , m_maxPos.y , m_minPos.z );
	vertices[9].position =	Vec3f(m_minPos.x , m_maxPos.y , m_minPos.z );
	vertices[10].position = Vec3f(m_minPos.x , m_maxPos.y , m_maxPos.z );
	vertices[11].position = Vec3f(m_maxPos.x , m_maxPos.y , m_maxPos.z );
											   		  			 
	vertices[12].position = Vec3f(m_minPos.x , m_maxPos.y , m_minPos.z );
	vertices[13].position = Vec3f(m_minPos.x , m_minPos.y , m_minPos.z );
	vertices[14].position = Vec3f(m_minPos.x , m_minPos.y , m_maxPos.z );
	vertices[15].position = Vec3f(m_minPos.x , m_maxPos.y , m_maxPos.z );
											   		  			
	vertices[16].position = Vec3f(m_minPos.x , m_minPos.y , m_minPos.z );
	vertices[17].position = Vec3f(m_maxPos.x , m_minPos.y , m_minPos.z );
	vertices[18].position = Vec3f(m_maxPos.x , m_minPos.y , m_maxPos.z );
	vertices[19].position = Vec3f(m_minPos.x , m_minPos.y , m_maxPos.z );
											   		   				 
	vertices[20].position = Vec3f(m_minPos.x , m_minPos.y , m_maxPos.z );
	vertices[21].position = Vec3f(m_maxPos.x , m_minPos.y , m_maxPos.z );
	vertices[22].position = Vec3f(m_maxPos.x , m_maxPos.y , m_maxPos.z );
	vertices[23].position = Vec3f(m_minPos.x , m_maxPos.y , m_maxPos.z );
	
	Vertex_PosColor e_vertices[24];

	e_vertices[0].position =  Vec3f(m_minPos.x , m_minPos.y , m_minPos.z );
	e_vertices[1].position =  Vec3f(m_maxPos.x , m_minPos.y , m_minPos.z );

	e_vertices[2].position =  Vec3f(m_maxPos.x , m_minPos.y , m_minPos.z );
	e_vertices[3].position =  Vec3f(m_maxPos.x , m_minPos.y , m_maxPos.z );

	e_vertices[4].position =  Vec3f(m_maxPos.x , m_minPos.y , m_maxPos.z );
	e_vertices[5].position =  Vec3f(m_minPos.x , m_minPos.y , m_maxPos.z );

	e_vertices[6].position =  Vec3f(m_minPos.x , m_minPos.y , m_maxPos.z );
	e_vertices[7].position =  Vec3f(m_minPos.x , m_minPos.y , m_minPos.z );

	e_vertices[8].position =  Vec3f(m_maxPos.x , m_minPos.y , m_minPos.z );
	e_vertices[9].position =  Vec3f(m_maxPos.x , m_maxPos.y , m_minPos.z );

	e_vertices[10].position = Vec3f(m_maxPos.x , m_maxPos.y , m_minPos.z );
	e_vertices[11].position = Vec3f(m_maxPos.x , m_maxPos.y , m_maxPos.z );

	e_vertices[12].position = Vec3f(m_maxPos.x , m_maxPos.y , m_maxPos.z );
	e_vertices[13].position = Vec3f(m_maxPos.x , m_minPos.y , m_maxPos.z );

	e_vertices[14].position = Vec3f(m_maxPos.x , m_maxPos.y , m_minPos.z );
	e_vertices[15].position = Vec3f(m_minPos.x , m_maxPos.y , m_minPos.z );

	e_vertices[16].position = Vec3f(m_minPos.x , m_maxPos.y , m_minPos.z );
	e_vertices[17].position = Vec3f(m_minPos.x , m_maxPos.y , m_maxPos.z );

	e_vertices[18].position = Vec3f(m_minPos.x , m_maxPos.y , m_maxPos.z );
	e_vertices[19].position = Vec3f(m_maxPos.x , m_maxPos.y , m_maxPos.z );

	e_vertices[20].position = Vec3f(m_minPos.x , m_minPos.y , m_maxPos.z );
	e_vertices[21].position = Vec3f(m_minPos.x , m_maxPos.y , m_maxPos.z );

	e_vertices[22].position = Vec3f(m_minPos.x , m_minPos.y , m_minPos.z );
	e_vertices[23].position = Vec3f(m_minPos.x , m_maxPos.y , m_minPos.z );

	for(int index = 0; index < 24; index++)
	{
		vertices[index].color = m_faceColor;
		e_vertices[index].color = m_edgeColor;
	}

	if(g_debugMode == DEPTH_TEST_OFF)
		OpenGLRenderer::DepthTest(false);

	if(g_debugMode == DEPTH_TEST_ON)
		OpenGLRenderer::DepthTest(true);

	glLineWidth(4.5f);
	OpenGLRenderer::DrawVertexWithVertexArray(vertices , 24 , GL_QUADS );
	OpenGLRenderer::DrawVertexWithVertexArray(e_vertices , 24 , GL_LINES );

	OpenGLRenderer::DepthTest(false);

	if(g_debugMode != DUAL_MODE)
		return;

	for(int index=0; index < 24; index++)
	{
		vertices[index].color *= 0.5f;
		e_vertices[index].color *= 0.5f;
	}

	glLineWidth(1.0f);
	OpenGLRenderer::DepthTest(false);
/*	OpenGLRenderer::DrawVectexWithVertexArray(vertices , 24 , GL_QUADS);*/
	OpenGLRenderer::DrawVertexWithVertexArray(e_vertices , 24 , GL_LINES);
	OpenGLRenderer::DepthTest(true);
}


void DebugDrawSphere::draw()
{
	if(g_debugMode == DEPTH_TEST_OFF)
		OpenGLRenderer::DepthTest(false);

	if(g_debugMode == DEPTH_TEST_ON)
		OpenGLRenderer::DepthTest(true);


	glLineWidth(4.5f);
	OpenGLRenderer::DrawWireSphere(m_center,m_radius,m_color,30,10);

	OpenGLRenderer::DepthTest(false);

	if(g_debugMode != DUAL_MODE)
		return;

	glLineWidth(1.0f);
	OpenGLRenderer::DepthTest(false);
	OpenGLRenderer::DrawWireSphere(m_center,m_radius,m_color*0.5f,30,10);
	OpenGLRenderer::DepthTest(true);
}

};