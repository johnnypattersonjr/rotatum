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

#ifndef  _WINDOWMANAGER_WIN32_WIN32WINDOW_
#define  _WINDOWMANAGER_WIN32_WIN32WINDOW_

#include <windows.h>
#include "windowManager/platformWindowMgr.h"
#include "windowManager/glfw/glfwWindow.h"
#include "gfx/gfxTarget.h"
#include "gfx/gfxStructs.h"
#include "sim/actionMap.h"

class Win32WindowManager;

/// Implementation of a window on Win32.
class Win32Window : public GLFWWindow
{
   friend class Win32WindowManager;

public:
   struct Accelerator
   {
      U32 mID;
      EventDescriptor mDescriptor;
   };
   typedef Vector<Accelerator> AcceleratorList;

private:
   typedef Vector<ACCEL> WinAccelList;

   /// @name Window Information
   ///
   /// @{

   /// The GLFW window procedure assigned at creation.
   WNDPROC mGLFWWindowProc;

   /// Windows HACCEL for accelerators
   HACCEL mAccelHandle;

   /// Keyboard accelerators for menus
   WinAccelList mWinAccelList;

   /// Menu associated with this window.  This is a passive property of the window and is not required to be used at all.
   HMENU mMenuHandle;

   /// @}

   /// Windows message handler callback.
   static LRESULT PASCAL WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

   /// Add an accelerator to the list of accelerators for this window. Intended for use by addAccelerators()
   void addAccelerator(Accelerator &accel);
   /// Remove an accelerator from the list of accelerators for this window. Intended for use by removeAccelerators()
   void removeAccelerator(Accelerator &accel);

public:
   Win32Window();
   ~Win32Window();

   /// Return the HWND (win32 window handle) for this window.
   HWND getHWND() const;

   HMENU &getMenuHandle()
   {
      return mMenuHandle;
   }

   void setMenuHandle(HMENU menuHandle);

   /// Add a list of accelerators to this window
   void addAccelerators(AcceleratorList &list);
   /// Remove a list of accelerators from this window
   void removeAccelerators(AcceleratorList &list);

   /// Returns true if @p info matches an accelerator
   bool isAccelerator(const InputEventInfo &info);

   /// Allow windows to translate messages. Used for accelerators.
   bool translateMessage(MSG &msg);

   virtual void setVideoMode(const GFXVideoMode &mode);
   
   virtual bool isVisible() const;
};
#endif
