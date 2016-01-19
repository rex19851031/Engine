#include "XBoxController.hpp"
#include <stdio.h>
#include <Windows.h>
#include <Xinput.h> // include the Xinput API
#include <cmath>
#pragma comment( lib, "xinput" ) // Link in the xinput.lib static library

namespace Henry
{

XBoxController::XBoxController()
{
	m_controllerNumber = 0;
}

//---------------------------------------------------------------------------
XBoxController::XBoxController(int ControllerNumber)
{
	m_controllerNumber = ControllerNumber;
	isRightTriggerThresholdCurrentlyPassed = false;
	isLeftTriggerThresholdCurrentlyPassed = false;
	isAPressedDown = false;
	isXPressedDown = false;
	isStartPressedDown = false;
	isBackPressedDown = false;
}


//---------------------------------------------------------------------------
XBoxController::~XBoxController(void)
{

}


//---------------------------------------------------------------------------
bool XBoxController::isControllerActive()
{
	XINPUT_STATE xboxControllerState;
	memset( &xboxControllerState, 0, sizeof( xboxControllerState ) );
	DWORD errorStatus = XInputGetState( m_controllerNumber, &xboxControllerState );
	if( errorStatus == ERROR_SUCCESS )
	{
		return true;
	}
	return false;
}


//---------------------------------------------------------------------------
bool XBoxController::isHexCurrentlyPressed(int hexButton)
{
	bool isDown = false;
	XINPUT_STATE xboxControllerState;
	memset( &xboxControllerState, 0, sizeof( xboxControllerState ) );
	DWORD errorStatus = XInputGetState( m_controllerNumber, &xboxControllerState );
	if( errorStatus == ERROR_SUCCESS )
	{
		isDown = ((xboxControllerState.Gamepad.wButtons & hexButton) != 0);
	}
	return isDown;
}


//---------------------------------------------------------------------------
bool XBoxController::isACurrentlyPressed()
{
	return isHexCurrentlyPressed(XINPUT_GAMEPAD_A);
}


//---------------------------------------------------------------------------
bool XBoxController::isBCurrentlyPressed()
{
	return isHexCurrentlyPressed(XINPUT_GAMEPAD_B);
}


//---------------------------------------------------------------------------
bool XBoxController::isXCurrentlyPressed()
{
	return isHexCurrentlyPressed(XINPUT_GAMEPAD_X);
}


//---------------------------------------------------------------------------
bool XBoxController::isYCurrentlyPressed()
{
	return isHexCurrentlyPressed(XINPUT_GAMEPAD_Y);
}


//---------------------------------------------------------------------------
bool XBoxController::isStartCurrentlyPressed()
{
	return isHexCurrentlyPressed(XINPUT_GAMEPAD_START);
}


//---------------------------------------------------------------------------
bool XBoxController::isBackCurrentlyPressed()
{
	return isHexCurrentlyPressed(XINPUT_GAMEPAD_BACK);
}


//---------------------------------------------------------------------------
double XBoxController::getControllerStickValue(int whichStickAndAxis)
{
	XINPUT_STATE xboxControllerState;
	memset( &xboxControllerState, 0, sizeof( xboxControllerState ) );
	DWORD errorStatus = XInputGetState( m_controllerNumber, &xboxControllerState );
	if( errorStatus == ERROR_SUCCESS )
	{
		if(whichStickAndAxis == LEFT_STICK_X)
		{
			return	xboxControllerState.Gamepad.sThumbLX; 
		}
		if(whichStickAndAxis == LEFT_STICK_Y)
		{
			return	xboxControllerState.Gamepad.sThumbLY; 
		}
		if(whichStickAndAxis == RIGHT_STICK_X)
		{
			return	xboxControllerState.Gamepad.sThumbRX; 
		}
		if(whichStickAndAxis == RIGHT_STICK_Y)
		{
			return	xboxControllerState.Gamepad.sThumbRY; 
		}
	}
	return 0.0;
}


//---------------------------------------------------------------------------
double XBoxController::getNormalizedControllerValue(int stickCase, double min, double max)
{
	double toReturn = 0.0;
	double getPos = getControllerStickValue(stickCase);

	if(abs(getPos) > max)
	{
		if(getPos < max)
		{
			getPos = -max;
		}
		else
		{
			getPos = max;
		}
	}
	if(abs(getPos) <= min)
	{
		return 0.0;
	}
	double minMaxDifference = max - min;
	toReturn = abs(getPos) - min;
	toReturn = toReturn/minMaxDifference;
	if(getPos < 0)
	{
		toReturn *= -1;
	}
	return toReturn;
}


//---------------------------------------------------------------------------
double XBoxController::getLeftStickX()
{
	return getNormalizedControllerValue(LEFT_STICK_X, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE - 1000, 30000);
}


//---------------------------------------------------------------------------
double XBoxController::getLeftStickY()
{
	return getNormalizedControllerValue(LEFT_STICK_Y, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE - 1000, 30000);
}


//---------------------------------------------------------------------------
double XBoxController::getRightStickX()
{
	return getNormalizedControllerValue(RIGHT_STICK_X, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE, 30000);
}


//---------------------------------------------------------------------------
double XBoxController::getRightStickY()
{
	return getNormalizedControllerValue(RIGHT_STICK_Y, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE, 30000);
}


//---------------------------------------------------------------------------
bool XBoxController::checkForLeftTrigger()
{
	if(!isLeftTriggerThresholdCurrentlyPassed)
	{
		if(isLeftTriggerAboveThreshold())
		{
			isLeftTriggerThresholdCurrentlyPassed = isLeftTriggerAboveThreshold();
			return true;
		}
	}
	isLeftTriggerThresholdCurrentlyPassed = isLeftTriggerAboveThreshold();
	return false;
}


//---------------------------------------------------------------------------
bool XBoxController::checkForRightTrigger()
{
	if(!isRightTriggerThresholdCurrentlyPassed)
	{
		if(isRightTriggerAboveThreshold())
		{
			isRightTriggerThresholdCurrentlyPassed = isRightTriggerAboveThreshold();
			return true;
		}
	}
	isRightTriggerThresholdCurrentlyPassed = isRightTriggerAboveThreshold();
	return false;
}


//---------------------------------------------------------------------------
bool XBoxController::isLeftTriggerAboveThreshold()
{
	double triggerThreshold = 125;
	bool isPressed = false;
	XINPUT_STATE xboxControllerState;
	memset( &xboxControllerState, 0, sizeof( xboxControllerState ) );
	DWORD errorStatus = XInputGetState( m_controllerNumber, &xboxControllerState );
	if( errorStatus == ERROR_SUCCESS )
	{
		if(xboxControllerState.Gamepad.bLeftTrigger >= triggerThreshold)
		{
			isPressed = true;
		}
	}
	return isPressed;
}


//---------------------------------------------------------------------------
bool XBoxController::isRightTriggerAboveThreshold()
{
	double triggerThreshold = 125;
	bool isPressed = false;
	XINPUT_STATE xboxControllerState;
	memset( &xboxControllerState, 0, sizeof( xboxControllerState ) );
	DWORD errorStatus = XInputGetState( m_controllerNumber, &xboxControllerState );
	if( errorStatus == ERROR_SUCCESS )
	{
		if(xboxControllerState.Gamepad.bRightTrigger >= triggerThreshold)
		{
			isPressed = true;
		}
	}
	return isPressed;
}


//---------------------------------------------------------------------------
bool XBoxController::isAPressedOnce()
{
	if(!isAPressedDown)
	{
		if(isACurrentlyPressed())
		{
			isAPressedDown = isACurrentlyPressed();
			return true;
		}
	}
	isAPressedDown = isACurrentlyPressed();
	return false;
}

//---------------------------------------------------------------------------
bool XBoxController::isXPressedOnce()
{
	if(!isXPressedDown)
	{
		if(isXCurrentlyPressed())
		{
			isXPressedDown = isXCurrentlyPressed();
			return true;
		}
	}
	isXPressedDown = isXCurrentlyPressed();
	return false;
}


//---------------------------------------------------------------------------
bool XBoxController::isStartPressedOnce()
{
	if(!isStartPressedDown)
	{
		if(isStartCurrentlyPressed())
		{
			isStartPressedDown = isStartCurrentlyPressed();
			return true;
		}
	}
	isStartPressedDown = isStartCurrentlyPressed();
	return false;
}


//---------------------------------------------------------------------------
bool XBoxController::isBackPressedOnce()
{
	if(!isBackPressedDown)
	{
		if(isBackCurrentlyPressed())
		{
			isBackPressedDown = isBackCurrentlyPressed();
			return true;
		}
	}
	isBackPressedDown = isBackCurrentlyPressed();
	return false;
}

};