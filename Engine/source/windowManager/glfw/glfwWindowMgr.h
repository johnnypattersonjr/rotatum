// Copyright (c) Johnny Patterson
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/util/tVector.h"
#include "windowManager/glfw/glfwWindow.h"
#include "windowManager/platformWindowMgr.h"

struct GLFWmonitor;

class GLFWWindowManager : public PlatformWindowManager
{
public:
	static void monitorCallback(GLFWmonitor* monitor, int event);

	GLFWWindowManager();
	virtual ~GLFWWindowManager();

	virtual S32 getDesktopBitDepth();
	virtual Point2I getDesktopResolution();
	virtual PlatformWindow* createWindow(GFXDevice* device, const GFXVideoMode& mode);
	virtual PlatformWindow* getFirstWindow();
	virtual PlatformWindow* getFocusedWindow();
	virtual PlatformWindow* getWindowById(WindowId id);
	virtual void getWindows(VectorPtr<PlatformWindow*>& windows);

protected:
	virtual bool setupWindow(PlatformWindow* window, GFXDevice* device, const GFXVideoMode& mode);

	void process();

	VectorPtr<GLFWWindow*> mWindowList;

	friend class GLFWWindow;
};
