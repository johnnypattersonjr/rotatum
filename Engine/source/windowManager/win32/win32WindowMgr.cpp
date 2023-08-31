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

#include "platformWin32/platformWin32.h"
#include "windowManager/win32/win32WindowMgr.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

// ------------------------------------------------------------------------

PlatformWindowManager * CreatePlatformWindowManager()
{
   return new Win32WindowManager();
}

// ------------------------------------------------------------------------

PlatformWindow *Win32WindowManager::createWindow(GFXDevice *device, const GFXVideoMode &mode)
{
   Win32Window *window = new Win32Window();
   window->mWindowId = getNextId();
   window->mTitle = getEngineProductString();

   if (!setupWindow(window, device, mode))
   {
      Con::errorf("Win32WindowManager::createWindow - Could not create window!");
      delete window;
      return NULL;
   }

   mWindowList.push_front(window);

   glfwShowWindow(window->mWindowHandle);

   return window;
}

bool Win32WindowManager::setupWindow(PlatformWindow* windowIn, GFXDevice *device, const GFXVideoMode &mode)
{
   bool setup = GLFWWindowManager::setupWindow(windowIn, device, mode);

   if (!setup)
      return false;

   Win32Window *window = (Win32Window *)windowIn;

   // Override GLFW window proc with Win32Window::WindowProc.
   window->mGLFWWindowProc = (WNDPROC)GetWindowLongPtr(window->getHWND(), GWLP_WNDPROC);
   SetWindowLongPtr(window->getHWND(), GWLP_WNDPROC, (LONG_PTR)Win32Window::WindowProc);

   return true;
}
