// Copyright (c) Johnny Patterson
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "glfwWindow.h"

#include "core/util/journal/process.h"
#include "windowManager/glfw/glfwWindowMgr.h"
#include "windowManager/glfw/glfwCursorController.h"
#include "windowManager/platformWindowMgr.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

GLFWWindow::GLFWWindow()
	: PlatformWindow()
	, mWindowHandle(NULL)
	, mDevice(NULL)
	, mTarget(NULL)
	, mFullscreen(false)
	, mMouseLocked(false)
	, mRenderOnRefresh(false)
	, mFocusCacheCursorVisible(true)
	, mFocusCacheMouseLocked(false)
{
	mCursorController = new GLFWCursorController(this);
}

GLFWWindow::~GLFWWindow()
{
	delete mCursorController;

	glfwDestroyWindow(mWindowHandle);

	((GLFWWindowManager*)WindowManager)->mWindowList.remove(this);
}

bool GLFWWindow::isFocused() const
{
	return glfwGetWindowAttrib(mWindowHandle, GLFW_FOCUSED) == GLFW_TRUE;
}

bool GLFWWindow::isMinimized() const
{
	return glfwGetWindowAttrib(mWindowHandle, GLFW_ICONIFIED) == GLFW_TRUE;
}

bool GLFWWindow::isMaximized() const
{
	return glfwGetWindowAttrib(mWindowHandle, GLFW_MAXIMIZED) == GLFW_TRUE;
}

bool GLFWWindow::isVisible() const
{
	return glfwGetWindowAttrib(mWindowHandle, GLFW_VISIBLE) == GLFW_TRUE && !isMinimized();
}

void GLFWWindow::minimize()
{
	glfwIconifyWindow(mWindowHandle);
}

void GLFWWindow::maximize()
{
	glfwMaximizeWindow(mWindowHandle);
}

void GLFWWindow::restore()
{
	glfwRestoreWindow(mWindowHandle);
}

void GLFWWindow::centerWindow()
{
	int currentX;
	int currentY;
	int currentWidth;
	int currentHeight;
	glfwGetWindowPos(mWindowHandle, &currentX, &currentY);
	glfwGetWindowSize(mWindowHandle, &currentWidth, &currentHeight);

	int centerX = currentX + (currentWidth >> 1);
	int centerY = currentY + (currentHeight >> 1);

	int count;
	GLFWmonitor** monitors = glfwGetMonitors(&count);

	int monitorX;
	int monitorY;
	int monitorWidth;
	int monitorHeight;
	int x;
	int y;

	for (int i = 0; i < count; ++i)
	{
		glfwGetMonitorWorkarea(monitors[i], &monitorX, &monitorY, &monitorWidth, &monitorHeight);

		if (monitorX > centerX || (monitorX + monitorWidth) < centerX ||
		    monitorY > centerY || (monitorY + monitorHeight) < centerY)
		{
			continue;
		}

		x = monitorX + ((monitorWidth - currentWidth) >> 1);
		y = monitorY + ((monitorHeight - currentHeight) >> 1);

		glfwSetWindowPos(mWindowHandle, x, y);
	}

	mVideoMode.resolution.set( currentWidth,currentHeight);

	if (mTarget.isValid())
		mTarget->resetMode();
}


void GLFWWindow::hide()
{
	glfwHideWindow(mWindowHandle);
}

void GLFWWindow::show()
{
	glfwShowWindow(mWindowHandle);
}

RectI GLFWWindow::getBounds() const
{
	int x;
	int y;
	int width;
	int height;
	glfwGetWindowPos(mWindowHandle, &x, &y);
	glfwGetWindowSize(mWindowHandle, &width, &height);
	return RectI(x, y, width, height);
}

Point2I GLFWWindow::getClientExtent() const
{
	int width;
	int height;
	glfwGetWindowSize(mWindowHandle, &width, &height);
	return Point2I(width, height);
}

void GLFWWindow::setFocus()
{
	glfwFocusWindow(mWindowHandle);
}

void GLFWWindow::setPosition(const Point2I& position)
{
	glfwSetWindowPos(mWindowHandle, position.x, position.y);
}

Point2I GLFWWindow::getPosition() const
{
	int x;
	int y;
	glfwGetWindowPos(mWindowHandle, &x, &y);
	return Point2I(x, y);
}

void GLFWWindow::setMinimumWindowSize(Point2I minSize)
{
	PlatformWindow::setMinimumWindowSize(minSize);

	if (!isSizeLocked())
	{
		glfwSetWindowSizeLimits(mWindowHandle,
		                        mMinimumSize.x ? mMinimumSize.x : GLFW_DONT_CARE,
		                        mMinimumSize.y ? mMinimumSize.y : GLFW_DONT_CARE,
		                        GLFW_DONT_CARE, GLFW_DONT_CARE);
	}
}

void GLFWWindow::setSize(const Point2I& size)
{
	glfwSetWindowSize(mWindowHandle, size.x, size.y);
	centerWindow();
}

void GLFWWindow::lockSize(bool locked)
{
	PlatformWindow::lockSize(locked);

	if (isSizeLocked())
	{
		glfwSetWindowSizeLimits(mWindowHandle, mLockedSize.x, mLockedSize.y, mLockedSize.x,
		                        mLockedSize.y);
	}
	else
	{
		glfwSetWindowSizeLimits(mWindowHandle,
		                        mMinimumSize.x ? mMinimumSize.x : GLFW_DONT_CARE,
		                        mMinimumSize.y ? mMinimumSize.y : GLFW_DONT_CARE,
		                        GLFW_DONT_CARE, GLFW_DONT_CARE);
	}
}

void GLFWWindow::setTitle(const char* title)
{
	mTitle = title;
	glfwSetWindowTitle(mWindowHandle, title);
}

void GLFWWindow::setMouseLocked(bool enable)
{
	if (enable == mMouseLocked)
		return;

	mMouseLocked = enable;

	if (enable)
	{
		glfwSetInputMode(mWindowHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetCursorPos(mWindowHandle, 0.0, 0.0);
	}
	else
	{
		glfwSetInputMode(mWindowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void GLFWWindow::setVideoMode(const GFXVideoMode& mode)
{
	mVideoMode = mode;
	mSuppressReset = true;

	if (mode.fullScreen)
	{
		if (GLFWmonitor* monitor = glfwGetPrimaryMonitor())
			glfwSetWindowMonitor(mWindowHandle, monitor, 0, 0, mVideoMode.resolution.x, mVideoMode.resolution.y, mVideoMode.refreshRate);
		else
			Con::errorf("Could not locate primary monitor.");

		if (mTarget.isValid())
			mTarget->resetMode();

		mFullscreen = true;
	}
	else
	{
		if (mTarget.isValid())
			mTarget->resetMode();

		glfwSetWindowMonitor(mWindowHandle, NULL, 0, 0, mVideoMode.resolution.x, mVideoMode.resolution.y, 0);
		setSize(mode.resolution);

		// glfwShowWindow(mWindowHandle);

		mFullscreen = false;
	}

	mSuppressReset = false;

	glfwFocusWindow(mWindowHandle);
}

void GLFWWindow::_setFullscreen(const bool fullscreen)
{
	if (fullscreen == mFullscreen)
		return;

	mFullscreen = fullscreen;
	if (fullscreen)
	{
		if (GLFWmonitor* monitor = glfwGetPrimaryMonitor())
		{
			const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
			glfwSetWindowMonitor(mWindowHandle, monitor, 0, 0, videoMode->width,
			                     videoMode->height, videoMode->refreshRate);
		}
		else
		{
			Con::errorf("Could not locate primary monitor.");
		}
	}
	else
	{
		glfwSetWindowMonitor(mWindowHandle, NULL, 0, 0, mVideoMode.resolution.x,
		                     mVideoMode.resolution.y, 0);
		setSize(mVideoMode.resolution);
	}
}

void GLFWWindow::charCallback(GLFWwindow* glfwWindow, unsigned int codepoint)
{
	GLFWWindow* window = (GLFWWindow*)glfwGetWindowUserPointer(glfwWindow);
	if (!window)
		return;

	// TODO: Use key instead of codepoint for shouldNotTranslate check.

	if (window->getKeyboardTranslation() &&
	    !window->shouldNotTranslate(Input::getModifierKeys(), codepoint))
	{
		window->charEvent.trigger(window->getWindowId(), Input::getModifierKeys(), codepoint);
	}
}

void GLFWWindow::cursorPositionCallback(GLFWwindow *glfwWindow, double x, double y)
{
	GLFWWindow* window = (GLFWWindow*)glfwGetWindowUserPointer(glfwWindow);
	if (!window)
		return;

	bool isLocked = window->isMouseLocked();

	window->mouseEvent.trigger(window->getWindowId(), Input::getModifierKeys(), x, y, isLocked);

	if (isLocked)
		glfwSetCursorPos(glfwWindow, 0.0, 0.0);
}

void GLFWWindow::keyCallback(GLFWwindow* glfwWindow, int key, int scancode, int action, int mods)
{
	GLFWWindow* window = (GLFWWindow*)glfwGetWindowUserPointer(glfwWindow);
	if (!window)
		return;

	U8 modifiers = 0;

	if (mods)
	{
		if (mods & GLFW_MOD_ALT)
			modifiers |= SI_ALT;
		if (mods & GLFW_MOD_CONTROL)
			modifiers |= SI_CTRL;
		if (mods & GLFW_MOD_SHIFT)
			modifiers |= SI_SHIFT;
		if (mods & GLFW_MOD_SUPER)
			modifiers |= SI_MAC_OPT;
	}

	Input::setModifierKeys(modifiers);

	window->keyEvent.trigger(window->getWindowId(), Input::getModifierKeys(), action, key);
}

void GLFWWindow::mouseButtonCallback(GLFWwindow* glfwWindow, int button, int action, int mods)
{
	GLFWWindow* window = (GLFWWindow*)glfwGetWindowUserPointer(glfwWindow);
	if (!window)
		return;

	window->buttonEvent.trigger(window->getWindowId(), Input::getModifierKeys(), action, button);
}

void GLFWWindow::scrollCallback(GLFWwindow* glfwWindow, double x, double y)
{
	GLFWWindow* window = (GLFWWindow*)glfwGetWindowUserPointer(glfwWindow);
	if (!window || (x == 0.0 && y == 0.0))
		return;

	const double kWin32WheelDelta = 120.0;
	window->wheelEvent.trigger(window->getWindowId(), Input::getModifierKeys(),
	                           x * kWin32WheelDelta, y * kWin32WheelDelta);
}

void GLFWWindow::windowCloseCallback(GLFWwindow *glfwWindow)
{
	GLFWWindow* window = (GLFWWindow*)glfwGetWindowUserPointer(glfwWindow);

	if (window)
	{
		window->appEvent.trigger(window->getWindowId(), WindowClose);
		window->appEvent.trigger(window->getWindowId(), WindowDestroy);
	}

	if (Journal::IsPlaying())
		Process::requestShutdown();
}

void GLFWWindow::windowFocusCallback(GLFWwindow *glfwWindow, int focused)
{
	GLFWWindow* window = (GLFWWindow*)glfwGetWindowUserPointer(glfwWindow);
	if (!window)
		return;

	Input::setModifierKeys(0);

	if (focused == GLFW_TRUE)
	{
		if (window->mFocusCacheMouseLocked)
			window->setMouseLocked(true);

		if (!window->mFocusCacheCursorVisible)
			window->setCursorVisible(false);

		window->appEvent.trigger(window->getWindowId(), GainFocus);

		if (!Input::isActive())
			Input::activate();
	}
	else
	{
		if (Input::isActive())
			Input::deactivate();

		window->mFocusCacheCursorVisible = window->isCursorVisible();
		window->mFocusCacheMouseLocked = window->isMouseLocked();

		if (window->mFocusCacheMouseLocked)
			window->setMouseLocked(false);

		if (!window->mFocusCacheCursorVisible)
			window->setCursorVisible(true);

		window->appEvent.trigger(window->getWindowId(), LoseFocus);
	}
}

void GLFWWindow::windowRefreshCallback(GLFWwindow *glfwWindow)
{
	GLFWWindow* window = (GLFWWindow*)glfwGetWindowUserPointer(glfwWindow);
	if (!window || !window->mRenderOnRefresh || Journal::IsDispatching())
		return;

	window->displayEvent.trigger(window->getWindowId());
}

void GLFWWindow::windowSizeCallback(GLFWwindow *glfwWindow, int width, int height)
{
	GLFWWindow* window = (GLFWWindow*)glfwGetWindowUserPointer(glfwWindow);
	if (!window || !width || !height)
		return;

	if (!Journal::IsPlaying())
	{
		window->resizeEvent.trigger(window->getWindowId(), width, height);

		if (window->isMouseLocked())
			window->setCursorPosition(width >> 1, height >> 1);
	}

	if (!window->mSuppressReset)
	{
		if (!window->mVideoMode.fullScreen)
			window->mVideoMode.resolution.set(width, height);

		if (window->getGFXTarget())
			window->getGFXTarget()->resetMode();
	}
}
