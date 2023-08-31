// Copyright (c) Johnny Patterson
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "windowManager/platformCursorController.h"

struct GLFWcursor;

class GLFWCursorController : public PlatformCursorController
{
public:
	GLFWCursorController(PlatformWindow* owner);
	~GLFWCursorController();

	virtual void setCursorPosition(S32 x, S32 y);
	virtual void getCursorPosition(Point2I& point);
	virtual void setCursorVisible(bool visible);
	virtual bool isCursorVisible();

	virtual void setCursorShape(U32 cursorID);
	virtual void setCursorShape(const UTF8* filename, bool reload);

	virtual U32 getDoubleClickTime();
	virtual S32 getDoubleClickWidth();
	virtual S32 getDoubleClickHeight();

private:
	GLFWcursor* mCursors[curCOUNT];
};
