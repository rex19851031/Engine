#pragma once

#ifndef DEBUGDRAWSHAPES_HPP
#define DEBUGDRAWSHAPES_HPP

#include "VertexStruct.hpp"

namespace Henry
{

enum DebugMode { DEPTH_TEST_ON , DEPTH_TEST_OFF , DUAL_MODE };
extern DebugMode g_debugMode;

class DebugDrawShapes
{
public:
	DebugDrawShapes(void) : m_durationSeconds(0.0f) , isDead(false){};
	~DebugDrawShapes(void){};
	virtual void draw(){};
	void update(double deltaSeconds) { m_durationSeconds -= deltaSeconds; if(m_durationSeconds <= 0.0f) isDead = true; };
	bool isDead;
	double m_durationSeconds;
};


class DebugDrawPosition : public DebugDrawShapes
{
public:
	DebugDrawPosition(Vec3f position,RGBA color,float radius,double duration = 0.0f) : m_position(position) , m_color(color) , m_radius(radius){ m_durationSeconds = duration; };
	~DebugDrawPosition(){};
	void draw();
	Vec3f m_position;

private:
	RGBA m_color;
	float m_radius;
};


class DebugDrawLine : public DebugDrawShapes
{
public:
	DebugDrawLine(Vec3f startPos,Vec3f endPos,RGBA startColor,RGBA endColor,double duration = 0.0f) : m_startPos(startPos) , m_endPos(endPos) , m_startColor(startColor) , m_endColor(endColor) { m_durationSeconds = duration; };
	~DebugDrawLine(){};
	void draw();
private:
	Vec3f m_startPos;
	Vec3f m_endPos;
	RGBA m_startColor;
	RGBA m_endColor;
};


class DebugDrawArrow : public DebugDrawShapes
{
public:
	DebugDrawArrow(Vec3f startPos,Vec3f endPos,RGBA startColor,RGBA endColor,double duration = 0.0f) : m_startPos(startPos) , m_endPos(endPos) , m_startColor(startColor) , m_endColor(endColor) { m_durationSeconds = duration; };
	~DebugDrawArrow(){};
	void draw();
private:
	Vec3f m_startPos;
	Vec3f m_endPos;
	RGBA m_startColor;
	RGBA m_endColor;
};


class DebugDrawAABB : public DebugDrawShapes
{
public:
	DebugDrawAABB(Vec3f minPos, Vec3f maxPos, RGBA edgeColor, RGBA faceColor,double duration = 0.0f) : m_minPos(minPos) , m_maxPos(maxPos) , m_edgeColor(edgeColor) , m_faceColor(faceColor) { m_durationSeconds = duration; };
	~DebugDrawAABB(){};
	void draw();
private:
	Vec3f m_minPos;
	Vec3f m_maxPos;
	RGBA m_edgeColor;
	RGBA m_faceColor;
};


class DebugDrawSphere : public DebugDrawShapes
{
public:
	DebugDrawSphere(Vec3f centerPos,RGBA color,float radius,double duration = 0.0f) : m_center(centerPos),m_color(color),m_radius(radius){ m_durationSeconds = duration; };
	~DebugDrawSphere(){};
	void draw();
private:
	Vec3f m_center;
	RGBA m_color;
	float m_radius;
};

};

#endif