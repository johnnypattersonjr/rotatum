// Copyright (c) Johnny Patterson
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "glfwWindowMgr.h"

#include "core/util/journal/process.h"
#include "gfx/gfxDevice.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

void GLFWWindowManager::monitorCallback(GLFWmonitor *monitor, int event)
{
	if (event != GLFW_CONNECTED)
		return;

	GLFWWindowManager* manager = (GLFWWindowManager*)WindowManager;
	const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
	int bitDepth = videoMode->blueBits + videoMode->greenBits + videoMode->redBits;

	for (VectorPtr<GLFWWindow*>::iterator it = manager->mWindowList.begin(); it != manager->mWindowList.end(); ++it)
	{
		GLFWWindow* window = *it;

		if (window->isVisible() && !window->mSuppressReset && window->getVideoMode().bitDepth != bitDepth)
			window->getGFXTarget()->resetMode();
	}
}

GLFWWindowManager::GLFWWindowManager()
	: PlatformWindowManager()
{
	glfwInit();

	glfwSetMonitorCallback(monitorCallback);

	Process::notify(this, &GLFWWindowManager::process, PROCESS_INPUT_ORDER);
}

GLFWWindowManager::~GLFWWindowManager()
{
	Process::remove(this, &GLFWWindowManager::process);

	while (!mWindowList.empty())
		delete mWindowList.back();

	glfwTerminate();
}

S32 GLFWWindowManager::getDesktopBitDepth()
{
	S32 bitDepth = -1;

	if (GLFWmonitor* monitor = glfwGetPrimaryMonitor())
	{
		const int alphaBits = 8;
		const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
		bitDepth = videoMode->blueBits + videoMode->greenBits + videoMode->redBits + alphaBits;
	}

	return bitDepth;
}

Point2I GLFWWindowManager::getDesktopResolution()
{
	Point2I resolution(-1, -1);

	if (GLFWmonitor* monitor = glfwGetPrimaryMonitor())
	{
		const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
		resolution.x = videoMode->width;
		resolution.y = videoMode->height;
	}

	return resolution;
}

PlatformWindow* GLFWWindowManager::createWindow(GFXDevice* device, const GFXVideoMode& mode)
{
	GLFWWindow* window = new GLFWWindow();

	if (!setupWindow(window, device, mode))
	{
		delete window;
		return NULL;
	}

	mWindowList.push_front(window);

	return window;
}

PlatformWindow* GLFWWindowManager::getFirstWindow()
{
	return !mWindowList.empty() ? mWindowList.front() : NULL;
}

PlatformWindow* GLFWWindowManager::getFocusedWindow()
{
	for (VectorPtr<GLFWWindow*>::iterator it = mWindowList.begin(); it != mWindowList.end(); ++it)
	{
		GLFWWindow* window = *it;

		if (window->isFocused())
			return window;
	}

	return NULL;
}

PlatformWindow* GLFWWindowManager::getWindowById(WindowId id)
{
	for (VectorPtr<GLFWWindow*>::iterator it = mWindowList.begin(); it != mWindowList.end(); ++it)
	{
		GLFWWindow* window = *it;

		if (window->getWindowId() == id)
			return window;
	}

	return NULL;
}

void GLFWWindowManager::getWindows(VectorPtr<PlatformWindow*>& windows)
{
	windows.merge(mWindowList);
}

bool GLFWWindowManager::setupWindow(PlatformWindow* windowIn, GFXDevice* device,
                                    const GFXVideoMode& mode)
{
	GLFWWindow* window = (GLFWWindow*)windowIn;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	// Create window as hidden in order to bind focus callback before shown.
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	GLFWwindow* glfwWindow = glfwCreateWindow(mode.resolution.x, mode.resolution.y,
	                                          window->mTitle.utf8(), NULL, NULL);

	if (glfwWindow == NULL)
		return false;

	window->mWindowHandle = glfwWindow;

	glfwSetWindowUserPointer(glfwWindow, window);

	glfwSetCharCallback(glfwWindow, GLFWWindow::charCallback);
	glfwSetCursorPosCallback(glfwWindow, GLFWWindow::cursorPositionCallback);
	glfwSetKeyCallback(glfwWindow, GLFWWindow::keyCallback);
	glfwSetMouseButtonCallback(glfwWindow, GLFWWindow::mouseButtonCallback);
	glfwSetScrollCallback(glfwWindow, GLFWWindow::scrollCallback);
	glfwSetWindowCloseCallback(glfwWindow, GLFWWindow::windowCloseCallback);
	glfwSetWindowFocusCallback(glfwWindow, GLFWWindow::windowFocusCallback);
	glfwSetWindowRefreshCallback(glfwWindow, GLFWWindow::windowRefreshCallback);
	glfwSetWindowSizeCallback(glfwWindow, GLFWWindow::windowSizeCallback);

	window->setVideoMode(mode);

	if (device)
	{
		window->mDevice = device;
		window->mTarget = device->allocWindowTarget(window);
	}

	return true;
}

void GLFWWindowManager::process()
{
	glfwPollEvents();

	for (VectorPtr<GLFWWindow*>::iterator it = mWindowList.begin(); it != mWindowList.end(); ++it)
	{
		(*it)->idleEvent.trigger();
	}
}
