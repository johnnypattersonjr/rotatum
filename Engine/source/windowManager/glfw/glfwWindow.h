// Copyright (c) Johnny Patterson
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "gfx/gfxStructs.h"
#include "gfx/gfxTarget.h"
#include "windowManager/platformWindow.h"

struct GLFWwindow;

class GLFWWindow : public PlatformWindow
{
public:
	GLFWWindow();
	virtual ~GLFWWindow();

	virtual bool isFocused() const;
	virtual bool isMinimized() const;
	virtual bool isMaximized() const;
	virtual bool isVisible() const;

	virtual void minimize();
	virtual void maximize();
	virtual void hide();
	virtual void show();
	virtual void restore();

	virtual void centerWindow();
	virtual RectI getBounds() const;
	virtual Point2I getClientExtent() const;
	virtual bool isFullscreen() { return mFullscreen; }
	virtual void setFocus();
	virtual void setPosition(const Point2I& position);
	virtual Point2I getPosition() const;
	virtual void lockSize(bool locked);
	virtual void setMinimumWindowSize(Point2I minSize);
	virtual void setSize(const Point2I& size);

	virtual void setTitle(const char* title);
	virtual const char* getTitle() const { return mTitle.utf8(); }

	virtual void setMouseLocked(bool enable);
	virtual bool isMouseLocked() const { return mMouseLocked; }
	virtual bool shouldLockMouse() const { return false; }

	virtual void* getPlatformDrawable() const { return mWindowHandle; }
	virtual WindowId getWindowId() { return mWindowId; };

	virtual GFXDevice* getGFXDevice() { return mDevice; }
	virtual GFXWindowTarget* getGFXTarget() { return mTarget; }
	virtual void setVideoMode(const GFXVideoMode& mode);
	virtual const GFXVideoMode& getVideoMode() const { return mVideoMode; }

	void setRenderOnRefresh(bool enable) { mRenderOnRefresh = enable; }

protected:
	virtual void _setFullscreen(const bool fullScreen);

	static void charCallback(GLFWwindow* glfwWindow, unsigned int codepoint);
	static void cursorPositionCallback(GLFWwindow* glfwWindow, double x, double y);
	static void keyCallback(GLFWwindow* glfwWindow, int key, int scancode, int action, int mods);
	static void mouseButtonCallback(GLFWwindow* glfwWindow, int button, int action, int mods);
	static void scrollCallback(GLFWwindow* glfwWindow, double x, double y);
	static void windowCloseCallback(GLFWwindow* glfwWindow);
	static void windowFocusCallback(GLFWwindow* glfwWindow, int focused);
	static void windowRefreshCallback(GLFWwindow* glfwWindow);
	static void windowSizeCallback(GLFWwindow* glfwWindow, int width, int height);

	GLFWwindow* mWindowHandle;

	String mTitle;

	GFXDevice* mDevice;
	GFXWindowTargetRef mTarget;
	GFXVideoMode mVideoMode;

	bool mFullscreen;
	bool mMouseLocked;
	bool mRenderOnRefresh;

	bool mFocusCacheCursorVisible;
	bool mFocusCacheMouseLocked;

	friend class GLFWCursorController;
	friend class GLFWWindowManager;
};
