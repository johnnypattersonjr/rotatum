//-----------------------------------------------------------------------------
// Copyright (c) Johnny Patterson
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef  _WINDOWMANAGER_WIN32_WIN32WINDOWMANAGER_
#define  _WINDOWMANAGER_WIN32_WIN32WINDOWMANAGER_

#include <windows.h>

#include "gfx/gfxStructs.h"
#include "windowManager/glfw/glfwWindowMgr.h"
#include "windowManager/win32/win32Window.h"

/// Win32 implementation of the window manager interface.
class Win32WindowManager : public GLFWWindowManager
{
   friend class Win32Window;

public:
   virtual PlatformWindow *createWindow(GFXDevice *device, const GFXVideoMode &mode);

protected:
   virtual bool setupWindow(PlatformWindow *window, GFXDevice *device, const GFXVideoMode &mode);
};

#endif