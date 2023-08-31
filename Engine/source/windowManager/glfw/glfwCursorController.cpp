// Copyright (c) Johnny Patterson
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "glfwCursorController.h"

#include "platform/platform.h"
#include "windowManager/glfw/glfwWindow.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#if defined(TORQUE_OS_WIN32)
#define NO_MINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

GLFWCursorController::GLFWCursorController(PlatformWindow* owner)
	: PlatformCursorController(owner)
{
	static S32 sStandardCursors[curCOUNT] = {
		GLFW_ARROW_CURSOR,
		GLFW_CROSSHAIR_CURSOR,
		GLFW_RESIZE_NS_CURSOR,
		GLFW_RESIZE_EW_CURSOR,
		GLFW_RESIZE_ALL_CURSOR,
		GLFW_IBEAM_CURSOR,
		GLFW_RESIZE_NESW_CURSOR,
		GLFW_RESIZE_NWSE_CURSOR,
		GLFW_POINTING_HAND_CURSOR,
	};

	for (S32 i = 0; i < curCOUNT; ++i)
		mCursors[i] = sStandardCursors[i] ? glfwCreateStandardCursor(sStandardCursors[i]) : NULL;

	pushCursor(curArrow);
}

GLFWCursorController::~GLFWCursorController()
{
	for (S32 i = 0; i < curCOUNT; ++i)
		if (GLFWcursor* cursor = mCursors[i])
			glfwDestroyCursor(cursor);
}

void GLFWCursorController::setCursorPosition(S32 x, S32 y)
{
	GLFWWindow* window = static_cast<GLFWWindow*>(mOwner);
	if (!window->mWindowHandle)
		return;

	glfwSetCursorPos(window->mWindowHandle, static_cast<double>(x), static_cast<double>(y));
}

void GLFWCursorController::getCursorPosition(Point2I& point)
{
	GLFWWindow* window = static_cast<GLFWWindow*>(mOwner);
	if (!window->mWindowHandle)
		return;

	double x;
	double y;
	glfwGetCursorPos(window->mWindowHandle, &x, &y);

	point.x = static_cast<S32>(x);
	point.y = static_cast<S32>(y);
}

void GLFWCursorController::setCursorVisible(bool visible)
{
	GLFWWindow* window = static_cast<GLFWWindow*>(mOwner);
	if (!window->mWindowHandle || visible == isCursorVisible())
		return;

	if (visible)
	{
		if (window->isMouseLocked())
		{
			glfwSetInputMode(window->mWindowHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwSetCursorPos(window->mWindowHandle, 0.0, 0.0);
		}
		else
		{
			glfwSetInputMode(window->mWindowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
	else
	{
		glfwSetInputMode(window->mWindowHandle, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	}
}

bool GLFWCursorController::isCursorVisible()
{
	GLFWWindow* window = static_cast<GLFWWindow*>(mOwner);
	if (!window->mWindowHandle)
		return false;

	return GLFW_CURSOR_DISABLED != glfwGetInputMode(window->mWindowHandle, GLFW_CURSOR);
}

void GLFWCursorController::setCursorShape(U32 cursorID)
{
	GLFWWindow* window = static_cast<GLFWWindow*>(mOwner);
	if (!window->mWindowHandle)
		return;

	glfwSetCursor(window->mWindowHandle, mCursors[mCursors[cursorID] ? cursorID : 0]);
}

void GLFWCursorController::setCursorShape(const UTF8* filename, bool reload)
{
	// TODO: Create and manage cursor
	AssertFatal(false, "Not implemented.");
}

U32 GLFWCursorController::getDoubleClickTime()
{
#if defined(TORQUE_OS_WIN32)
	return GetDoubleClickTime();
#else
	return 500;
#endif
}

S32 GLFWCursorController::getDoubleClickWidth()
{
#if defined(TORQUE_OS_WIN32)
	return GetSystemMetrics(SM_CXDOUBLECLK);
#else
	return 4;
#endif
}

S32 GLFWCursorController::getDoubleClickHeight()
{
#if defined(TORQUE_OS_WIN32)
	return GetSystemMetrics(SM_CYDOUBLECLK);
#else
	return 4;
#endif
}
