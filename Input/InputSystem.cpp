#include "InputSystem.hpp"

#include <Windows.h>

namespace Henry
{

InputSystem* _inputSystem = nullptr;


LRESULT CALLBACK InputSystem_WndProc( HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam )
{
	unsigned char asKey = (unsigned char) wParam;
	switch( wmMessageCode )
	{
	case WM_ACTIVATE:
		_inputSystem->m_isFocus = asKey == 1 ? true : false;
		break;
	case WM_CLOSE:
		// fake input escape
		_inputSystem->m_keyState[255].isHold = true;
		_inputSystem->m_keyState[255].isPressedFrame = true;
		_inputSystem->m_keyState[Inputs::INPUT_ESCAPE].isHold = true;
		_inputSystem->m_keyState[Inputs::INPUT_ESCAPE].isPressedFrame = true;
		break;
	case WM_QUIT:
		return 0;
	case WM_KEYDOWN:
		if(!_inputSystem->m_keyState[asKey].isHold)
			_inputSystem->m_keyState[asKey].isPressedFrame = true;
		_inputSystem->m_keyState[asKey].isHold = true;
		break;
	case WM_KEYUP:
		_inputSystem->m_keyState[asKey].isHold = false;
		_inputSystem->m_keyState[asKey].isPressedFrame = false;
		break;
	case WM_CHAR:
		_inputSystem->m_chars.push_back( (char)asKey );
		break;

	case WM_LBUTTONDOWN:
		if(!_inputSystem->m_mouseState[Inputs::MOUSE_LB].isHold)
			_inputSystem->m_mouseState[Inputs::MOUSE_LB].isPressedFrame = true;
		_inputSystem->m_mouseState[Inputs::MOUSE_LB].isHold = true;
		break;					   
	case WM_RBUTTONDOWN:		   
		if(!_inputSystem->m_mouseState[Inputs::MOUSE_RB].isHold)
			_inputSystem->m_mouseState[Inputs::MOUSE_RB].isPressedFrame = true;
		_inputSystem->m_mouseState[Inputs::MOUSE_RB].isHold = true;
		break;					   
	case WM_MBUTTONDOWN:		   
		if (!_inputSystem->m_mouseState[Inputs::MOUSE_WHEEL].isHold)
			_inputSystem->m_mouseState[Inputs::MOUSE_WHEEL].isPressedFrame = true;
		_inputSystem->m_mouseState[Inputs::MOUSE_WHEEL].isHold = true;
		break;					   
	case WM_LBUTTONUP:			   
		_inputSystem->m_mouseState[Inputs::MOUSE_LB].isHold = false;
		_inputSystem->m_mouseState[Inputs::MOUSE_LB].isReleasedFrame = true;
		break;					   
	case WM_RBUTTONUP:			   
		_inputSystem->m_mouseState[Inputs::MOUSE_RB].isHold = false;
		_inputSystem->m_mouseState[Inputs::MOUSE_RB].isReleasedFrame = true;
		break;					   
	case WM_MBUTTONUP:			   
		_inputSystem->m_mouseState[Inputs::MOUSE_WHEEL].isHold = false;
		_inputSystem->m_mouseState[Inputs::MOUSE_WHEEL].isReleasedFrame = true;
		break;
	case WM_MOUSEWHEEL:
		if(GET_WHEEL_DELTA_WPARAM(wParam) > 0)
			_inputSystem->m_mouseWheelValue = 1;
		else
			_inputSystem->m_mouseWheelValue = -1;
		break;
	}

	return DefWindowProc( windowHandle, wmMessageCode, wParam, lParam );
}


InputSystem::InputSystem(void* platformHandle) : m_platformHandle(platformHandle)
{
	_inputSystem = this;
	m_isFocus = true;
	m_showCursor = false;

	HWND hwnd = (HWND)m_platformHandle;
	SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG)InputSystem_WndProc);
	
	RECT rect;
	if (GetClientRect(hwnd,&rect)) //(GetWindowRect(hwnd, &rect))
	{
		m_windowSize.x = rect.right - rect.left;
		m_windowSize.y = rect.bottom - rect.top;
		m_centerPoint.x = m_windowSize.x >> 1;
		m_centerPoint.y = m_windowSize.y >> 1;
	}

	for(int index=0; index <= 255; index++)
	{
		m_keyState[index].isHold = false;
		m_keyState[index].isPressedFrame = false;
		m_keyState[index].isReleasedFrame = false;
	}

	for(int index=0; index<3; index++)
	{
		m_mouseState[index].isHold = false;
		m_mouseState[index].isPressedFrame = false;
		m_mouseState[index].isReleasedFrame = false;
	}

	m_mouseWheelValue = 0;
	m_chars.reserve(100);

	SetCursorPos(m_centerPoint.x, m_centerPoint.y);
}


InputSystem::~InputSystem(void)
{
}


void InputSystem::Update()
{
	for(int index=0; index <= 255; index++)
	{
		m_keyState[index].isPressedFrame = false;
		m_keyState[index].isReleasedFrame = false;
	}

	for(int index=0; index < 3; index++)
	{
		m_mouseState[index].isPressedFrame = false;
		m_mouseState[index].isReleasedFrame = false;
	}

	m_mouseWheelValue = 0;
	m_chars = m_empty;
	
	RunMessagePump();
}


void InputSystem::RunMessagePump()
{
	MSG queuedMessage;
	for( ;; )
	{
		const BOOL wasMessagePresent = PeekMessage( &queuedMessage, NULL, 0, 0, PM_REMOVE );
		if( !wasMessagePresent )
		{
			break;
		}

		TranslateMessage( &queuedMessage );
		DispatchMessage( &queuedMessage );
	}
}


void InputSystem::ToggleCursorDisplay(bool visible)
{
	m_showCursor = visible;

	if (visible)
		while (ShowCursor(true) <= 0);
	else
		while (ShowCursor(false) >= 0); /*{ SetCursorPos(m_centerPoint.x, m_centerPoint.y); };*/
}


bool InputSystem::PressedOnce(const unsigned char keyValue)
{
	return m_keyState[keyValue].isPressedFrame;
}


Vec2i InputSystem::GetMouseMovementFromLastFrame()
{
	POINT cursorPos;
	GetCursorPos(&cursorPos);

	if (!m_isFocus || m_showCursor)
		return Vec2i(0, 0);
		
	SetCursorPos(m_centerPoint.x, m_centerPoint.y);
	Vec2i mouseDeltas(cursorPos.x - m_centerPoint.x, cursorPos.y - m_centerPoint.y);

	return mouseDeltas;
}


void InputSystem::CenterMouseCursor()
{
	SetCursorPos(m_centerPoint.x, m_centerPoint.y);
}


void InputSystem::GetMousePosition(Vec2f* position)
{
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	ScreenToClient((HWND)m_platformHandle, &cursorPos);
	*position = Vec2f((float)cursorPos.x, m_windowSize.y - (float)cursorPos.y);
}


};