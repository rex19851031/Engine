#pragma once

#ifndef XBOXCONTROLLER_HPP
#define XBOXCONTROLLER_HPP

namespace Henry
{

class XBoxController
{
public:
	XBoxController();
	XBoxController(int ControllerNumber);
	~XBoxController(void);

	bool isHexCurrentlyPressed(int hexButton);
	bool isXCurrentlyPressed();
	bool isYCurrentlyPressed();
	bool isBCurrentlyPressed();
	bool isACurrentlyPressed();
	bool isStartCurrentlyPressed();
	bool isBackCurrentlyPressed();
	double getLeftStickX();
	double getLeftStickY();
	double getControllerStickValue(int whichStickAndAxis);
	double getNormalizedControllerValue(int stickCase, double min, double max);

	double getRightStickX();
	double getRightStickY();

	bool isControllerActive();

	bool checkForRightTrigger();
	bool checkForLeftTrigger();

	bool isLeftTriggerAboveThreshold();
	bool isRightTriggerAboveThreshold();

	bool isAPressedOnce();
	bool isXPressedOnce();
	bool isStartPressedOnce();
	bool isBackPressedOnce();

	enum stickCases 
	{
		LEFT_STICK_X,
		LEFT_STICK_Y,
		RIGHT_STICK_X,
		RIGHT_STICK_Y
	};

	int m_controllerNumber;

	bool isRightTriggerThresholdCurrentlyPassed;
	bool isLeftTriggerThresholdCurrentlyPassed;

	bool isAPressedDown;
	bool isXPressedDown;
	bool isStartPressedDown;
	bool isBackPressedDown;
};

};

#endif